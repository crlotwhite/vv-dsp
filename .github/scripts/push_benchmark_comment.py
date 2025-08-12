#!/usr/bin/env python3
"""
Script to process benchmark results and post a comment to GitHub PR.
This follows the common pattern used by many projects for benchmark comparison.
"""

import json
import os
import sys
import argparse
from typing import List
from dataclasses import dataclass


@dataclass
class BenchmarkResult:
    """Represents a single benchmark result."""
    name: str
    cpu_time: float
    real_time: float
    time_unit: str
    iterations: int
    bytes_per_second: float = 0.0
    items_per_second: float = 0.0


def parse_benchmark_json(file_path: str) -> List[BenchmarkResult]:
    """Parse Google Benchmark JSON output."""
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    results = []
    for benchmark in data.get('benchmarks', []):
        if benchmark.get('run_type') == 'iteration':  # Skip aggregate results
            result = BenchmarkResult(
                name=benchmark['name'],
                cpu_time=benchmark['cpu_time'],
                real_time=benchmark['real_time'],
                time_unit=benchmark['time_unit'],
                iterations=benchmark['iterations'],
                bytes_per_second=benchmark.get('bytes_per_second', 0.0),
                items_per_second=benchmark.get('items_per_second', 0.0)
            )
            results.append(result)

    return results


def format_time(time_ns: float, unit: str) -> str:
    """Format time with appropriate units."""
    if unit == 'ns':
        if time_ns >= 1e9:
            return f"{time_ns / 1e9:.3f} s"
        elif time_ns >= 1e6:
            return f"{time_ns / 1e6:.3f} ms"
        elif time_ns >= 1e3:
            return f"{time_ns / 1e3:.3f} μs"
        else:
            return f"{time_ns:.3f} ns"
    return f"{time_ns:.3f} {unit}"


def format_throughput(rate: float) -> str:
    """Format throughput with appropriate units."""
    if rate >= 1e9:
        return f"{rate / 1e9:.2f} G/s"
    elif rate >= 1e6:
        return f"{rate / 1e6:.2f} M/s"
    elif rate >= 1e3:
        return f"{rate / 1e3:.2f} K/s"
    else:
        return f"{rate:.2f} /s"


def generate_benchmark_comment(
    results: List[BenchmarkResult],
    commit_sha: str
) -> str:
    """Generate a markdown comment for the PR."""

    # Group results by benchmark family
    families = {}
    for result in results:
        if '/' in result.name:
            family_name = result.name.split('/')[0]
        else:
            family_name = result.name
        if family_name not in families:
            families[family_name] = []
        families[family_name].append(result)

    comment = f"""## 🚀 Benchmark Results
*Commit: {commit_sha[:8]}*

"""

    for family_name, family_results in families.items():
        comment += f"### {family_name}\n\n"
        comment += "| Test Case | CPU Time | Real Time | Throughput |\n"
        comment += "|-----------|----------|-----------|------------|\n"

        for result in family_results[:10]:  # Limit to top 10 per family
            cpu_time = format_time(result.cpu_time, result.time_unit)
            real_time = format_time(result.real_time, result.time_unit)

            throughput = ""
            if result.items_per_second > 0:
                throughput = format_throughput(result.items_per_second)
            elif result.bytes_per_second > 0:
                throughput = format_throughput(result.bytes_per_second) + "B"

            if '/' in result.name:
                test_name = result.name.replace(family_name + '/', '')
            else:
                test_name = result.name

            comment += (
                f"| {test_name} | {cpu_time} | {real_time} | "
                f"{throughput} |\n"
            )

        if len(family_results) > 10:
            comment += f"*... and {len(family_results) - 10} more tests*\n"

        comment += "\n"

    avg_cpu_time = sum(r.cpu_time for r in results) / len(results)
    fastest = min(results, key=lambda x: x.cpu_time)
    slowest = max(results, key=lambda x: x.cpu_time)

    comment += f"""
<details>
<summary>📊 Performance Insights</summary>

- **Total benchmarks run**: {len(results)}
- **Benchmark families**: {len(families)}
- **Average CPU time**: {avg_cpu_time:.2f} ns
- **Fastest test**: {fastest.name} ({format_time(fastest.cpu_time, 'ns')})
- **Slowest test**: {slowest.name} ({format_time(slowest.cpu_time, 'ns')})

</details>

*Benchmark results are automatically generated on every PR.
Click on the GitHub Actions tab to see the full benchmark run.*
"""

    return comment


def main():
    parser = argparse.ArgumentParser(
        description='Generate benchmark comment for GitHub PR'
    )
    parser.add_argument(
        '--results-file',
        required=True,
        help='Path to benchmark results JSON'
    )
    parser.add_argument(
        '--commit-sha',
        required=True,
        help='Git commit SHA'
    )
    parser.add_argument(
        '--output',
        help='Output file for comment (default: stdout)'
    )

    args = parser.parse_args()

    if not os.path.exists(args.results_file):
        print(f"Error: Results file {args.results_file} not found",
              file=sys.stderr)
        sys.exit(1)

    try:
        results = parse_benchmark_json(args.results_file)
        if not results:
            print("Warning: No benchmark results found", file=sys.stderr)
            sys.exit(0)

        comment = generate_benchmark_comment(results, args.commit_sha)

        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(comment)
            print(f"Comment written to {args.output}")
        else:
            print(comment)

    except (json.JSONDecodeError, KeyError, FileNotFoundError) as e:
        print(f"Error processing benchmark results: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
