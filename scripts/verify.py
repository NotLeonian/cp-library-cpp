from __future__ import annotations

from collections import OrderedDict
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import os
from pathlib import Path
import threading
from typing import Dict, Iterable, List

import onlinejudge_verify.languages.list
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

_download_lock = threading.Lock()


def load_json(path: Path) -> Dict[str, str]:
    if not path.exists():
        return {}
    with path.open() as fh:
        return json.load(fh)


def dump_json(path: Path, data: Dict[str, str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w") as fh:
        json.dump(data, fh, indent=4, sort_keys=True)


def patch_atcoder_to_sample_only() -> None:
    """
    online-judge-verify-helper は oj download --system を固定で呼ぶ。
    AtCoder の system testcase は Dropbox 側に依存するため、AtCoder だけ sample-only に落とす。
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

            # 並列実行時に同じ problem URL の download が競合しないようにする。
            # -d <dir> が既に埋まっていれば、他 worker が download 済みなので skip する。
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
    同じ PROBLEM URL の test を別 worker に分けると、
    .verify-helper/cache/<hash>/a.out が競合し得る。
    そのため bucket 分けでは PROBLEM URL 単位でまとめる。
    """
    language = onlinejudge_verify.languages.list.get(path)
    if language is None:
        return f"__unsupported__:{path}"

    try:
        attrs = language.list_attributes(path, basedir=ROOT)
    except Exception:
        # 実際の verify で失敗させる。ここでは bucket 分けだけ続行する。
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
    worker ごとに timestamp file を分ける。
    対象 bucket の test だけ seed して、Marker の初期化コストと競合を抑える。
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


def main() -> None:
    patch_atcoder_to_sample_only()

    base_timestamp = load_json(TIMESTAMP_FILE)
    tests = list_test_files()
    buckets = make_buckets(tests, BUCKET_SIZE)

    print(f"tests: {len(tests)}")
    print(f"buckets: {len(buckets)}")
    print(f"max workers: {MAX_WORKERS}")
    print("AtCoder verification mode: sample-only")

    merged_timestamp = dict(base_timestamp)

    WORKER_TIMESTAMP_DIR.mkdir(parents=True, exist_ok=True)

    with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
        futures = [
            executor.submit(run_bucket, index, bucket, base_timestamp)
            for index, bucket in enumerate(buckets)
        ]

        for future in as_completed(futures):
            # ここで例外を main thread に伝播させる。
            merged_timestamp.update(future.result())

    dump_json(TIMESTAMP_FILE, merged_timestamp)


if __name__ == "__main__":
    main()
