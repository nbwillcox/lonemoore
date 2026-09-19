"""Analyze seven native adventure captures; prior room-kit results are context only.

No game is launched and no settings/saves are written. A PASS evidence record
requires all seven scenes, fresh successful capture metadata and verified hashes.
"""
import argparse
import csv
import hashlib
import json
import math
import statistics
from datetime import datetime, timezone
from pathlib import Path

csv.field_size_limit(20_000_000)
ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "Saved/AdventurePolish"
CANDIDATE = ROOT / "Builds/AdventurePolishCandidate/Windows"
EXPECTED = {
    "Floor_02_Entrance": (1, "Entrance"),
    "Floor_07_Entrance": (6, "Entrance"),
    "Floor_07_BossApproach": (6, "BossApproach"),
    "Floor_07_Bridge": (6, "Bridge"),
    "Floor_09_BossApproach": (8, "BossApproach"),
    "Floor_13_Entrance": (12, "Entrance"),
    "Floor_17_Entrance": (16, "Entrance"),
}
LIMITATIONS = (
    "These are warmed stationary offscreen captures, not sustained gameplay or loading-hitch measurements. "
    "Previous RoomKitPolish Release scenes are contextual comparisons only: authored floors changed from "
    "104 to 68 sections, altering layouts, content placement and visible work. Matching scene labels do "
    "not make the workloads identical or establish an optimization speedup. Monitor tearing is not measured."
)


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def finite_number(value):
    number = float(value)
    if not math.isfinite(number):
        raise ValueError("Nonfinite performance metric")
    return number


def percentile(values, fraction):
    ordered = sorted(values)
    # Same index convention as the prior RoomKitPolish analyzer.
    return ordered[min(int(len(ordered) * fraction), len(ordered) - 1)]


def analyze_scene(directory, scene, executable, executable_hash):
    floor, native_scene = EXPECTED[scene]
    metadata_path = directory / f"{scene}.capture.json"
    capture = read_json(metadata_path)
    assert capture["status"] == "PASS" and capture["scene"] == scene, "Capture did not pass"
    assert capture["floorIndex"] == floor and capture["nativeScene"] == native_scene, "Wrong native scene"
    assert capture["authoredSections"] == 68 and capture["processExitCode"] == 0, "Wrong generation or process failure"
    assert capture["installedSettingsUnchanged"] is True, "Installed settings preservation missing"
    assert Path(capture["candidateExecutable"]).resolve() == executable.resolve(), "Wrong candidate executable"
    assert capture["candidateExecutableSha256"].lower() == executable_hash, "Candidate changed after capture"
    source_hashes = {str(metadata_path.relative_to(ROOT)): sha256(metadata_path)}
    for kind, suffix in (("csv", "csv"), ("screenshot", "png"), ("log", "log")):
        path = directory / f"{scene}.{suffix}"
        assert Path(capture["sources"][kind]["path"]).resolve() == path.resolve(), "Wrong capture source"
        digest = sha256(path)
        assert capture["sources"][kind]["sha256"].lower() == digest, f"{kind} changed after capture"
        source_hashes[str(path.relative_to(ROOT))] = digest
    log = (directory / f"{scene}.log").read_text(encoding="utf-8-sig", errors="replace")
    assert f"ROOM_KIT_BENCHMARK_READY floor={floor} scene={native_scene} rooms=68 frames=2400" in log, "Native benchmark marker missing"
    assert "CSV finalize time" in log, "CSV capture did not finalize"
    assert str(executable.parent).replace("\\", "/") + "/" in log.replace("\\", "/"), "Log is not from the candidate"
    assert not any(marker in log for marker in ("Fatal error:", "Assertion failed:", "GPU Crashed", "ROOM_KIT_BENCHMARK_FAILED")), "Native failure logged"
    rows = []
    with (directory / f"{scene}.csv").open(encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream):
            try:
                if finite_number(row["FrameTime"]) > 0:
                    rows.append(row)
            except (ValueError, TypeError, KeyError):
                pass
    total_frames = len(rows)
    rows = rows[-1500:-200]
    assert len(rows) == 1300, f"Expected 1300 warmed samples, got {len(rows)}"
    times = [finite_number(row["FrameTime"]) for row in rows]
    mean = statistics.mean(times)
    p95, p99 = percentile(times, .95), percentile(times, .99)
    stats = {}
    for key in ("GameThreadTime", "RenderThreadTime", "GPUTime", "GPU/TemporalSuperResolution", "GPU/RenderDeferredLighting", "GPU/LumenSceneUpdate", "GPU/LumenScreenProbeGather"):
        values = []
        for row in rows:
            try:
                value = finite_number(row[key])
                if value >= 0:
                    values.append(value)
            except (ValueError, TypeError, KeyError):
                pass
        if values:
            stats[key] = round(statistics.mean(values), 3)
    assert stats.get("GPUTime", 0) > 0, "Native GPU timings missing"
    result = {
        "label": "Release", "scene": scene, "samples": len(times), "totalCapturedFrames": total_frames,
        "fps": round(1000 / mean, 1), "mean_ms": round(mean, 3),
        "p95_ms": round(p95, 3), "p95_fps": round(1000 / p95, 1),
        "p99_ms": round(p99, 3), "p99_fps": round(1000 / p99, 1),
        "gpu_mean_ms": stats["GPUTime"], "stats_ms": stats, "outputResolution": capture["outputResolution"],
    }
    return result, source_hashes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--label", choices=["Release"], default="Release")
    args = parser.parse_args()
    directory = EVIDENCE / "Performance" / args.label
    executable = CANDIDATE / "DungeonCrawler/Binaries/Win64/DungeonCrawler.exe"
    errors, results, sources = [], [], {}
    executable_hash = sha256(executable) if executable.is_file() else ""
    if not executable_hash:
        errors.append("AdventurePolishCandidate executable is missing.")
    for scene in sorted(EXPECTED):
        try:
            result, hashes = analyze_scene(directory, scene, executable, executable_hash)
            results.append(result)
            sources.update(hashes)
            print(json.dumps(result))
        except (OSError, AssertionError, ValueError, KeyError, TypeError) as exc:
            errors.append(f"{scene}: {exc}")
    prior_path = ROOT / "Saved/RoomKitPolish/performance_results.json"
    prior = {}
    try:
        prior = {row["scene"]: row for row in read_json(prior_path) if row["label"] == "Release"}
        sources[str(prior_path.relative_to(ROOT))] = sha256(prior_path)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        errors.append(f"Previous contextual results: {exc}")
    comparisons = []
    for current in results:
        previous = prior.get(current["scene"])
        if not previous:
            errors.append(f"Previous representative scene missing: {current['scene']}")
            continue
        comparisons.append({
            "scene": current["scene"], "comparisonType": "contextual_changed_layout", "identicalWorkload": False,
            "previousFps": previous["fps"], "currentFps": current["fps"],
            "fpsDifferencePercent": round((current["fps"] / previous["fps"] - 1) * 100, 3),
            "previousMeanMs": previous["mean_ms"], "currentMeanMs": current["mean_ms"],
            "previousP95Ms": previous.get("p95_ms"), "currentP95Ms": current["p95_ms"],
            "previousP99Ms": previous["p99_ms"], "currentP99Ms": current["p99_ms"],
            "previousGpuMeanMs": previous.get("gpu_mean_ms", previous.get("stats_ms", {}).get("GPUTime")),
            "currentGpuMeanMs": current["gpu_mean_ms"],
        })
    if len({tuple(row["outputResolution"]) for row in results}) > 1:
        errors.append("Scenes were captured at different output resolutions.")
    status = "PASS" if not errors and len(results) == len(EXPECTED) else "FAIL"
    generated = datetime.now(timezone.utc).isoformat()
    summary = {
        "status": status, "generatedUtc": generated, "sceneCount": len(results), "expectedSceneCount": 7,
        "expectedScenes": sorted(EXPECTED), "samplesPerScene": 1300,
        "candidate": str(CANDIDATE), "candidateExecutableSha256": executable_hash,
        "currentAuthoredSections": 68, "previousAuthoredSections": 104,
        "comparisonType": "contextual_changed_layout", "identicalWorkload": False,
        "fpsRange": [min(row["fps"] for row in results), max(row["fps"] for row in results)] if results else [],
        "p95MsRange": [min(row["p95_ms"] for row in results), max(row["p95_ms"] for row in results)] if results else [],
        "p99MsRange": [min(row["p99_ms"] for row in results), max(row["p99_ms"] for row in results)] if results else [],
        "gpuMeanMsRange": [min(row["gpu_mean_ms"] for row in results), max(row["gpu_mean_ms"] for row in results)] if results else [],
        "outputResolution": results[0]["outputResolution"] if results else [],
        "method": "Uncapped offscreen stationary native scenes; eight-second warm-up; 2400 captured frames; analyze 1300 central warmed samples using the prior percentile convention.",
        "installedSettingsUnchanged": not errors and len(results) == 7,
        "limitations": LIMITATIONS, "errors": errors, "sources": sources,
    }
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    for filename, payload in (
        ("performance_results.json", results),
        ("performance_comparison.json", {**summary, "scenes": comparisons}),
        ("performance_release_evidence.json", {**summary, "scenes": results}),
    ):
        (EVIDENCE / filename).write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(json.dumps({"status": status, "scenes": len(results), "errors": errors, "comparisonType": summary["comparisonType"]}))
    return 0 if status == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
