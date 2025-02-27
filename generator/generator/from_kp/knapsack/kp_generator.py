from .kp_instance import KPInstance
from dataclasses import dataclass
from typing import List, Optional
import abc


@dataclass
class KPGenerator(metaclass=abc.ABCMeta):
    range_ub: Optional[int]


    @abc.abstractmethod
    def generate(self, num_items: int, instance_num: int, instance_tot: int) -> KPInstance:
        pass
    
    def generate_all(self, num_items: int, instance_tot: int) -> List[KPInstance]:
        instances = list()

        # for instance_num in range(1, instance_tot + 1):
        # We drastically reduce the number of instances by generating
        # ~0.1 * instance_tot of them instead of instance_tot.
        for instance_num in range(int(0.45 * instance_tot), int(0.55 * instance_tot)):
            instances.append(self.generate(num_items=num_items, instance_num=instance_num, instance_tot=instance_tot))

        return instances
