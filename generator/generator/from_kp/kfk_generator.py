from .knapsack.kp_generator import KPGenerator
from .knapsack.kp_instance import KPInstance
from ..instance import Instance
from dataclasses import dataclass
from typing import List
import abc


# KFK: Kpgf From Kp


@dataclass
class KFKGenerator(metaclass=abc.ABCMeta):
    kp_generator: KPGenerator
    num_items: int
    num_classes: int


    @abc.abstractmethod
    def assign_items_to_classes(self, kp_instance: KPInstance) -> List[int]:
        pass

    @abc.abstractmethod
    def generate_resource(self, kp_instance: KPInstance, klass: List[int]) -> Instance:
        pass

    def extend_kp(self, kp_instance: KPInstance) -> Instance:
        klass = self.assign_items_to_classes(kp_instance=kp_instance)
        return self.generate_resource(kp_instance=kp_instance, klass=klass)


    def generate_all(self, instance_tot: int) -> List[Instance]:
        return [
            self.extend_kp(kp_instance=kp_instance)
            for kp_instance in self.kp_generator.generate_all(num_items=self.num_items, instance_tot=instance_tot)
        ]


class AssignClassesMixin:
    def assign_items_to_classes(self, kp_instance: KPInstance) -> List[int]:
        pass


class GenerateResourceMixin:
    def generate_resource(self, kp_instance: KPInstance, klass: List[int]) -> Instance:
        pass