from .kp_instance import KPInstance
from .kp_generator import KPGenerator
from random import randint


class InverseStronglyCorrelatedGenerator(KPGenerator):
    def generate(self, num_items: int, instance_num: int, instance_tot: int) -> KPInstance:
        assert 1 <= instance_num <= instance_tot

        profit = [randint(1, self.range_ub) for _ in range(num_items)]
        weight = [int(p + self.range_ub / 10) for p in profit]
        tot_w = sum(weight)
        capacity = int(tot_w * instance_num / (instance_tot + 1))

        return KPInstance(num_items=num_items, capacity=capacity, profit=profit, weight=weight)
