from __future__ import annotations
from dataclasses import dataclass
from typing import List


@dataclass
class Instance:
    num_items: int
    num_classes: int
    capacity: int
    resource_lb: List[int]
    resource_ub: List[int]
    profit: List[int]
    weight: List[int]
    resource: List[int]
    klass: List[int] # Class of each item


    def to_file(self, filename: str) -> None:
        kl, p, w, r = self.klass, self.profit, self.weight, self.resource

        # Items are written contigously by class. I.e., first all items of
        # the first class, then all those of the second class, etc.
        kl, p, w, r = zip(*sorted(zip(kl, p, w, r)))

        with open(filename, mode='w') as f:
            f.write(f"{self.num_items} {self.num_classes} {self.capacity}\n")

            for k in range(self.num_classes):
                items = [j for j in range(self.num_items) if self.klass[j] == k]
                ni = len(items)
                f.write(f"{ni} {self.resource_lb[k]} {self.resource_ub[k]}\n")

            for x, y, z in zip(p, w, r):
                f.write(f"{x} {y} {z}\n")

    @staticmethod
    def from_file(filename: str) -> Instance:
        with open(filename) as f:
            num_items, num_classes, capacity = map(int, f.readline().split())
            
            ni, h_lb, h_ub = list(), list(), list()
            for _ in range(num_classes):
                x, y, z = map(int, f.readline().split())
                ni.append(x)
                h_lb.append(y)
                h_ub.append(z)

            p, w, h = list(), list(), list()
            for _ in range(num_items):
                x, y, z = map(int, f.readline().split())
                p.append(x)
                w.append(y)
                h.append(z)

            klass = list()
            for klass_n, klass_sz in enumerate(ni):
                for _ in range(klass_sz):
                    klass.append(klass_n)

            return Instance(num_items=num_items, num_classes=num_classes, capacity=capacity, resource_lb=h_lb, resource_ub=h_ub,
                            profit=p, weight=w, resource=h, klass=klass)