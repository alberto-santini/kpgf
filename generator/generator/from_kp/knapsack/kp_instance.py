from dataclasses import dataclass
from typing import List


@dataclass
class KPInstance:
    num_items: int
    capacity: int
    profit: List[int]
    weight: List[int]