from __future__ import annotations

import argparse
from collections import OrderedDict
from concurrent.futures import ThreadPoolExecutor, as_completed
import contextlib
import json
import os
from pathlib import Path
import shutil
import tempfile
import threading
from typing import Dict, Iterable, Iterator, List

import onlinejudge_verify.config
import onlinejudge_verify.documentation.main
import onlinejudge_verify.languages.list
import onlinejudge_verify.main as oj_verify_main
import onlinejudge_verify.marker
import onlinejudge_verify.verify


ROOT = Path.cwd()
VERIFY_HELPER_DIR = ROOT / ".verify-helper"

TIMESTAMP_FILE = (
    VERIFY_HELPER_DIR
    / f"timestamps.{'remote' if 'GITHUB_ACTION' in os.environ else 'local'}.json"
)
WORKER_TIMESTAMP_DIR = VERIFY_HELPER_DIR / "parallel-timestamps"

BUCKET_SIZE = int(os.environ.get("VERIFY_BUCKET_SIZE", "20"))
MAX_WORKERS = int(os.environ.get("VERIFY_JOBS", str(os.cpu_count() or 2)))
VERIFY_TIMEOUT = float(os.environ.get("VERIFY_TIMEOUT", "6000"))
TLE = float(os.environ.get("VERIFY_TLE", "60"))
OJ_TEST_JOBS = int(os.environ.get("OJ_TEST_JOBS", "1"))
DOCS_JOBS = int(os.environ.get("VERIFY_DOCS_JOBS", os.environ.get("VERIFY_JOBS", "1")))
DEFAULT_BRANCH = os.environ.get("VERIFY_DEFAULT_BRANCH", "main")
TIMESTAMP_JSON_INDENT = 4

_download_lock = threading.Lock()


DEFAULT_DOCS_EXCLUDE = [
    Path("scripts"),
    Path("ac-library"),
]


def parse_path_list_env(name: str, default: List[Path]) -> List[Path]:
    raw = os.environ.get(name)
    if raw is None:
        return list(default)

    paths: List[Path] = []
    for line in raw.replace(",", "\n").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        paths.append(Path(line))
    return paths


def to_relative_path(path: Path, *, basedir: Path) -> Path:
    path = Path(path)
    try:
        return path.resolve().relative_to(basedir.resolve())
    except Exception:
        return path


def docs_excluded_paths(*, basedir: Path) -> List[Path]:
    excluded = parse_path_list_env("VERIFY_DOCS_EXCLUDE", DEFAULT_DOCS_EXCLUDE)

    # docs config 側に exclude がある場合はそれも考慮する
    try:
        site_config = onlinejudge_verify.documentation.main.load_render_config(
            basedir=basedir,
        )
        excluded.extend(Path(p) for p in site_config.config_yml.get("exclude", []))
    except Exception:
        pass

    # 順序は維持して重複を削除
    seen = set()
    result: List[Path] = []
    for p in excluded:
        key = str(p)
        if key in seen:
            continue
        seen.add(key)
        result.append(p)
    return result


def patch_docs_source_scan() -> None:
    """
    oj-verify docs の source code statistics / dependency graph の起点から
    docs の対象ではない補助スクリプトや submodule を除外する。

    これにより scripts/*.py の dependency graph timeout や
    ac-library/test および ac-library/tools 由来の解析失敗を避ける
    """
    from onlinejudge_verify.documentation import configure as cfg

    if getattr(cfg, "_cp_library_cpp_source_scan_patched", False):
        return

    original_find_source_code_paths = cfg._find_source_code_paths

    def patched_find_source_code_paths(*, basedir: Path):
        basedir = Path(basedir)
        paths = original_find_source_code_paths(basedir=basedir)
        excluded_paths = docs_excluded_paths(basedir=basedir)

        if not excluded_paths:
            return paths

        return [
            path
            for path in paths
            if not cfg.is_excluded(
                to_relative_path(Path(path), basedir=basedir),
                excluded_paths=excluded_paths,
            )
        ]

    cfg._find_source_code_paths = patched_find_source_code_paths
    cfg._cp_library_cpp_source_scan_patched = True


def check_documentation_of_consistency() -> None:
    """
    Markdown Front Matter の documentation_of が
    存在する source code を指しているかを生成前に検査する
    """
    from onlinejudge_verify.documentation import configure
    from onlinejudge_verify.documentation import front_matter as oj_front_matter
    from onlinejudge_verify.languages import list as language_list

    basedir = ROOT.resolve()
    excluded_paths = docs_excluded_paths(basedir=basedir)

    bad = []
    checked_markdown = 0
    checked_documentation_of = 0

    for md in sorted(
        configure.find_markdown_paths(basedir=basedir),
        key=lambda p: str(p),
    ):
        md_abs = md.resolve()
        rel_md = md_abs.relative_to(basedir)
        checked_markdown += 1

        front_matter, _ = oj_front_matter.split_front_matter(md_abs.read_bytes())
        documentation_of = front_matter.get("documentation_of")

        if documentation_of is None:
            continue

        checked_documentation_of += 1

        if not isinstance(documentation_of, str):
            bad.append(
                (
                    rel_md,
                    documentation_of,
                    "documentation_of must be a string",
                )
            )
            continue

        src = configure.resolve_documentation_of(
            documentation_of,
            markdown_path=rel_md,
            basedir=basedir,
        )

        if src is None:
            bad.append(
                (
                    rel_md,
                    documentation_of,
                    "source file is not found",
                )
            )
            continue

        rel_src = src.resolve().relative_to(basedir)

        if configure.is_excluded(rel_src, excluded_paths=excluded_paths):
            bad.append(
                (
                    rel_md,
                    documentation_of,
                    f"points to excluded source: {rel_src}",
                )
            )
            continue

        if language_list.get(rel_src) is None:
            bad.append(
                (
                    rel_md,
                    documentation_of,
                    f"points to a file that oj-verify docs does not treat as source code: {rel_src}",
                )
            )
            continue

    print(f"checked markdown files: {checked_markdown}")
    print(f"checked documentation_of entries: {checked_documentation_of}")

    if excluded_paths:
        print("docs exclude entries: " + ", ".join(map(str, excluded_paths)))

    if not bad:
        return

    print("invalid documentation_of entries:")
    for md, documentation_of, reason in bad:
        print(f"{md}: documentation_of={documentation_of!r}: {reason}")

    raise RuntimeError("documentation_of consistency check failed")


@contextlib.contextmanager
def docs_cxx_wrapper() -> Iterator[None]:
    """
    oj-verify docs / bundle でのみ発生する非本質な warning を抑制する
    """
    real_gxx = shutil.which("g++")
    if real_gxx is None:
        raise RuntimeError("g++ is not found")

    old_path = os.environ.get("PATH", "")
    old_real_gxx = os.environ.get("REAL_GXX")
    old_cxx = os.environ.get("CXX")

    with tempfile.TemporaryDirectory(prefix="oj-docs-cxx-wrapper-") as tmpdir:
        wrapper_dir = Path(tmpdir)
        wrapper = wrapper_dir / "g++"

        wrapper.write_text(
            """#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${REAL_GXX:-}" ]]; then
  echo "REAL_GXX is not set" >&2
  exit 127
fi

has_fpreprocessed=0
has_dD=0
has_E=0

for arg in "$@"; do
  case "$arg" in
    -fpreprocessed) has_fpreprocessed=1 ;;
    -dD) has_dD=1 ;;
    -E) has_E=1 ;;
  esac
done

if [[ "${has_fpreprocessed}${has_dD}${has_E}" == "111" ]]; then
  exec "${REAL_GXX}" -w "$@"
else
  exec "${REAL_GXX}" "$@"
fi
""",
            encoding="utf-8",
        )
        wrapper.chmod(0o755)

        os.environ["PATH"] = f"{wrapper_dir}{os.pathsep}{old_path}"
        os.environ["REAL_GXX"] = real_gxx
        os.environ["CXX"] = str(wrapper)

        try:
            yield
        finally:
            os.environ["PATH"] = old_path

            if old_real_gxx is None:
                os.environ.pop("REAL_GXX", None)
            else:
                os.environ["REAL_GXX"] = old_real_gxx

            if old_cxx is None:
                os.environ.pop("CXX", None)
            else:
                os.environ["CXX"] = old_cxx


def is_truthy(value: str | None) -> bool:
    return (value or "").lower() in {"1", "true", "yes", "y", "on"}


def load_json(path: Path) -> Dict[str, str]:
    if not path.exists():
        return {}
    with path.open(encoding="utf-8") as fh:
        return json.load(fh)


def dump_json(path: Path, data: Dict[str, str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(
            data,
            indent=TIMESTAMP_JSON_INDENT,
            sort_keys=True,
            ensure_ascii=False,
        )
        + "\n",
        encoding="utf-8",
    )


def normalize_timestamp_file(path: Path = TIMESTAMP_FILE) -> None:
    if path.exists():
        dump_json(path, load_json(path))


def setup_verify_helper_config() -> None:
    onlinejudge_verify.config.set_config_path(
        Path(onlinejudge_verify.config.default_config_path)
    )


def drop_verify_cache() -> None:
    """
    full verification のために timestamp とローカル verify cache を捨てる
    """
    TIMESTAMP_FILE.unlink(missing_ok=True)
    shutil.rmtree(VERIFY_HELPER_DIR / "cache", ignore_errors=True)
    shutil.rmtree(WORKER_TIMESTAMP_DIR, ignore_errors=True)


def patch_atcoder_to_sample_only() -> None:
    """
    online-judge-verify-helper は oj download --system を固定で呼ぶので
    AtCoder だけ sample-only とする
    """
    original_exec_command = onlinejudge_verify.verify.exec_command

    def patched_exec_command(command: List[str]):
        command = list(map(str, command))

        is_download = (
            len(command) >= 2 and command[0] == "oj" and command[1] == "download"
        )

        if is_download:
            is_atcoder = any(arg.startswith("https://atcoder.jp/") for arg in command)

            if is_atcoder and "--system" in command:
                command.remove("--system")

            # 並列実行時に同じ problem URL の download が競合しないようにする
            # -d <dir> が既に埋まっていれば、他の worker がダウンロード済みなのでスキップする
            output_dir = None
            if "-d" in command:
                i = command.index("-d")
                if i + 1 < len(command):
                    output_dir = Path(command[i + 1])

            with _download_lock:
                if (
                    output_dir is not None
                    and output_dir.exists()
                    and any(output_dir.iterdir())
                ):
                    return
                return original_exec_command(command)

        return original_exec_command(command)

    onlinejudge_verify.verify.exec_command = patched_exec_command


def problem_key(path: Path) -> str:
    """
    同じ PROBLEM URL の test を別の worker に分けると、
    .verify-helper/cache/<hash>/a.out が競合し得る

    そのため PROBLEM URL 単位で bucket を分ける
    """
    language = onlinejudge_verify.languages.list.get(path)
    if language is None:
        return f"__unsupported__:{path}"

    try:
        attrs = language.list_attributes(path, basedir=ROOT)
    except Exception:
        # 実際の verify で失敗させる
        # ここでは bucket 分割だけ続行する
        return f"__attribute_error__:{path}"

    return attrs.get("PROBLEM") or f"__no_problem__:{path}"


def list_test_files() -> List[Path]:
    return sorted(Path("test").glob("**/*.test.cpp"))


def make_buckets(paths: Iterable[Path], bucket_size: int) -> List[List[Path]]:
    groups: "OrderedDict[str, List[Path]]" = OrderedDict()

    for path in paths:
        groups.setdefault(problem_key(path), []).append(path)

    buckets: List[List[Path]] = [[]]

    for group in groups.values():
        if buckets[-1] and len(buckets[-1]) + len(group) > bucket_size:
            buckets.append([])
        buckets[-1].extend(group)

    return [bucket for bucket in buckets if bucket]


def seed_worker_timestamp(
    worker_timestamp_file: Path, tests: List[Path], base_timestamp: Dict[str, str]
) -> None:
    """
    worker ごとに timestamp file を分ける
    対象となる bucket の test だけ seed して、
    マーカーの初期化コストと競合を抑える。
    """
    seeded = {
        str(path): base_timestamp[str(path)]
        for path in tests
        if str(path) in base_timestamp
    }
    dump_json(worker_timestamp_file, seeded)


def run_bucket(
    index: int, tests: List[Path], base_timestamp: Dict[str, str]
) -> Dict[str, str]:
    worker_timestamp_file = WORKER_TIMESTAMP_DIR / f"timestamps.{index}.json"
    seed_worker_timestamp(worker_timestamp_file, tests, base_timestamp)

    use_git_timestamp = "GITHUB_ACTION" in os.environ

    with onlinejudge_verify.marker.VerificationMarker(
        json_path=worker_timestamp_file,
        use_git_timestamp=use_git_timestamp,
    ) as marker:
        summary = onlinejudge_verify.verify.main(
            tests,
            marker=marker,
            timeout=VERIFY_TIMEOUT,
            tle=TLE,
            jobs=OJ_TEST_JOBS,
        )

    summary.show()

    if not summary.succeeded():
        failed = ", ".join(map(str, summary.failed_test_paths))
        raise RuntimeError(f"verification failed in bucket {index}: {failed}")

    return load_json(worker_timestamp_file)


def run_verification(*, full_verify: bool = False) -> None:
    setup_verify_helper_config()
    patch_atcoder_to_sample_only()

    full_verify = full_verify or is_truthy(os.environ.get("VERIFY_FULL"))
    if full_verify:
        print("Full verification: ignore timestamp/cache")
        drop_verify_cache()

    base_timestamp = load_json(TIMESTAMP_FILE)
    tests = list_test_files()
    buckets = make_buckets(tests, BUCKET_SIZE)

    print(f"tests: {len(tests)}")
    print(f"buckets: {len(buckets)}")
    print(f"max workers: {MAX_WORKERS}")
    print("AtCoder verification mode: sample-only")
    print(f"full verification: {full_verify}")

    merged_timestamp = dict(base_timestamp)
    shutil.rmtree(WORKER_TIMESTAMP_DIR, ignore_errors=True)
    WORKER_TIMESTAMP_DIR.mkdir(parents=True, exist_ok=True)

    with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
        futures = [
            executor.submit(run_bucket, index, bucket, base_timestamp)
            for index, bucket in enumerate(buckets)
        ]

        for future in as_completed(futures):
            # ここで例外を main thread に渡す
            merged_timestamp.update(future.result())

    dump_json(TIMESTAMP_FILE, merged_timestamp)


def deploy_docs() -> None:
    setup_verify_helper_config()

    if "GITHUB_ACTION" in os.environ:
        expected_ref = f"refs/heads/{DEFAULT_BRANCH}"
        actual_ref = os.environ.get("GITHUB_REF", "")
        if actual_ref != expected_ref:
            print(
                "Updating GitHub Pages is skipped: "
                f'this execution is on "{actual_ref}", not "{expected_ref}".'
            )
            return

        if not os.environ.get("GH_PAT"):
            raise RuntimeError("GH_PAT is required to deploy documents to GitHub Pages")

    if is_truthy(os.environ.get("VERIFY_DOCS_CHECK_DOCUMENTATION_OF", "1")):
        check_documentation_of_consistency()

    if is_truthy(os.environ.get("VERIFY_DOCS_PATCH_SOURCE_SCAN", "1")):
        patch_docs_source_scan()

    oj_verify_main.generate_gitignore()

    docs_context = (
        docs_cxx_wrapper()
        if is_truthy(os.environ.get("VERIFY_DOCS_WRAP_CXX", "1"))
        else contextlib.nullcontext()
    )

    with docs_context:
        print("generate documents...")
        onlinejudge_verify.documentation.main.main(jobs=DOCS_JOBS)

    if "GITHUB_ACTION" in os.environ:
        print("upload documents...")
        oj_verify_main.push_documents_to_gh_pages(
            src_dir=VERIFY_HELPER_DIR / "markdown",
        )
    else:
        print(f"documents generated at {VERIFY_HELPER_DIR / 'markdown'}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "command",
        nargs="?",
        choices=("run", "docs", "normalize-timestamps"),
        default="run",
        help="run verification, generate/deploy documentation, or normalize timestamp JSON",
    )
    parser.add_argument(
        "--full",
        action="store_true",
        help="ignore timestamp/cache and re-run all verification",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    if args.command == "run":
        run_verification(full_verify=args.full)
    elif args.command == "docs":
        deploy_docs()
    elif args.command == "normalize-timestamps":
        normalize_timestamp_file()


if __name__ == "__main__":
    main()
