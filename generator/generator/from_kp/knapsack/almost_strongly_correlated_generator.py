from .kp_instance import KPInstance
from .kp_generator import KPGenerator
from random import randint


class AlmostStronglyCorrelatedGenerator(KPGenerator):
    def generate(self, num_items: int, instance_num: int, instance_tot: int) -> KPInstance:
        assert 1 <= instance_num <= instance_tot

        weight = [randint(1, self.range_ub) for _ in range(num_items)]
        profit = [
            randint(
                int(w + self.range_ub / 10 - self.range_ub / 500),
                int(w + self.range_ub / 10 + self.range_ub / 500)
            ) for w in weight
        ]
        tot_w = sum(weight)
        capacity = int(tot_w * instance_num / (instance_tot + 1))

        return KPInstance(num_items=num_items, capacity=capacity, profit=profit, weight=weight)
