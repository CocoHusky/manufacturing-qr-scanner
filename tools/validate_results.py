"""Summarize independent scanner presentations from a CSV file."""
import argparse
import csv
import statistics
from collections import defaultdict

def summarize(rows):
    groups = defaultdict(list)
    for r in rows:
        groups[(r['engine'], r['symbol_id'])].append(r)
    result = []
    for (engine, symbol), trials in sorted(groups.items()):
        correct = [r for r in trials if r['outcome'] == 'correct']
        times = [float(r['latency_ms']) for r in correct if r['latency_ms']]
        wrong = sum(r['outcome'] == 'wrong' for r in trials)
        n = len(trials)
        result.append(dict(engine=engine, symbol_id=symbol, trials=n,
                           correct=len(correct), wrong=wrong,
                           success_pct=round(100*len(correct)/n, 2),
                           median_ms=statistics.median(times) if times else '',
                           screen_pass=n>=20 and len(correct)/n>=.95 and wrong==0))
    return result

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('csv')
    a = p.parse_args()
    with open(a.csv, newline='') as f:
        for r in summarize(csv.DictReader(f)):
            print(r)
