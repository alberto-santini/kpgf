from .kp_instance import KPInstance
from .kp_generator import KPGenerator
from .uncorrelated_generator import UncorrleatedGenerator
from .weakly_correlated_generator import WeaklyCorrelatedGenerator
from .strongly_correlated_generator import StronglyCorrelatedGenerator
from dataclasses import dataclass
from typing import Type
from math import ceil
from random import randint


@dataclass
class SpannerGenerator(KPGenerator):
    base_generator: Type
    num_spanners: int
    range_mult: int

    def __get_spanner(self, num_items: int, instance_num: int, instance_tot: int) -> KPInstance:
        assert self.base_generator in (UncorrleatedGenerator, WeaklyCorrelatedGenerator, StronglyCorrelatedGenerator)
        instance = self.base_generator(range_ub=self.range_ub).generate(num_items=num_items, instance_num=instance_num, instance_tot=instance_tot)

        # Normalise the instance
        instance.weight = [int(ceil(2 * w / self.range_mult)) for w in instance.weight]
        instance.profit = [int(ceil(2 * p / self.range_mult)) for p in instance.profit]

        return instance

    def generate(self, num_items: int, instance_num: int, instance_tot: int) -> KPInstance:
        assert 1 <= instance_num <= instance_tot

        spanner = self.__get_spanner(num_items=self.num_spanners, instance_num=instance_num, instance_tot=instance_tot)
        weight = list()
        profit = list()

        for _ in range(num_items):
            k = randint(0, self.num_spanners - 1)
            mult = randint(1, self.range_mult)
            weight.append(mult * spanner.weight[k])
            profit.append(mult * spanner.profit[k])

        tot_w = sum(weight)
        capacity = int(tot_w * instance_num / (instance_tot + 1))

        return KPInstance(num_items=num_items, capacity=capacity, profit=profit, weight=weight)
