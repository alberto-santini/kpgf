from typing import List
from os.path import realpath
from ..instance import Instance
from .kfk_generator import AssignClassesMixin, GenerateResourceMixin, KFKGenerator
from .knapsack.kp_instance import KPInstance
from .knapsack.almost_strongly_correlated_generator import AlmostStronglyCorrelatedGenerator
from .knapsack.circle_generator import CircleGenerator
from .knapsack.inverse_strongly_correlated_generator import InverseStronglyCorrelatedGenerator
from .knapsack.mstr_generator import MSTRGenerator
from .knapsack.profit_ceiling_generator import ProfitCeilingGenerator
from .knapsack.spanner_generator import SpannerGenerator
from .knapsack.strongly_correlated_generator import StronglyCorrelatedGenerator
from .knapsack.subset_sum_generator import SubsetSumGenerator
from .knapsack.uncorrelated_generator import UncorrleatedGenerator
from .knapsack.uncorrelated_similar_generator import UncorrleatedSimilarGenerator
from .knapsack.weakly_correlated_generator import WeaklyCorrelatedGenerator
import random


class AssignClassesRandomUniform(AssignClassesMixin):
    def assign_items_to_classes(self, kp_instance: KPInstance) -> List[int]:
        assert kp_instance.num_items == self.num_items
        assert self.num_items >= self.num_classes

        # Ensures that there is no empty class

        n_times = self.num_items // self.num_classes
        reminder = self.num_items % self.num_classes
        klass = list(range(self.num_classes)) * n_times + list(range(reminder))
        random.shuffle(klass)

        return klass


class GenerateResourcesRandomUniform(GenerateResourceMixin):
    def generate_resource(self, kp_instance: KPInstance, klass: List[int]) -> Instance:
        assert kp_instance.num_items == self.num_items

        weight_ratio = sum(kp_instance.weight) / kp_instance.capacity

        resource_lb = [random.randint(50, 150) for _ in range(self.num_classes)]
        resource_ub = [random.randint(150, 250) for _ in range(self.num_classes)]
        class_sz = [
            len([j for j in range(self.num_items) if klass[j] == k])
            for k in range(self.num_classes)
        ]
        resource = [
            min(
                int(weight_ratio * random.randint(125, 175) / class_sz[klass[j]]),
                resource_ub[klass[j]]
            )
            for j in range(self.num_items)
        ]
        class_tot_resource = [
            sum(resource[j] for j in range(self.num_items) if klass[j] == k)
            for k in range(self.num_classes)
        ]

        for k, (rlb, tr) in enumerate(zip(resource_lb, class_tot_resource)):
            if tr < rlb:
                print(f"Weight ratio: {weight_ratio}, class sz: {class_sz[k]}")
                print(f"Class {k}: [" + ', '.join(str(j) for j in range(self.num_items) if klass[j] == k) + ']')
                print(f"Resources: [" + ', '.join(str(resource[j]) for j in range(self.num_items) if klass[j] == k) + ']')
                raise RuntimeError(f"Class {k}. Total resource: {tr}. Resource LB: {rlb}.")

        return Instance(
            num_items=self.num_items, num_classes=self.num_classes,
            capacity=kp_instance.capacity, resource_lb=resource_lb,
            resource_ub=resource_ub, profit=kp_instance.profit,
            weight=kp_instance.weight, resource=resource, klass=klass)


if __name__ == '__main__':
    pisinger_ranges = (1000, 10_000)

    kp_generators = {
        "uncorrelated_similar": UncorrleatedSimilarGenerator(range_ub=None) # Range UB ignored because the range is hard-coded for uncorrelated similar
    }

    for rng in pisinger_ranges:
        kp_generators |= {
            f"almost_strongly_correlated_{rng}": AlmostStronglyCorrelatedGenerator(range_ub=rng),
            f"circle_{rng}": CircleGenerator(range_ub=rng, coeff=2/3),
            f"inverse_strongly_correlated_{rng}": InverseStronglyCorrelatedGenerator(range_ub=rng),
            f"mstr_{rng}": MSTRGenerator(range_ub=rng, fixed_profit=(300, 200), divider=6),
            f"profit_ceiling_{rng}": ProfitCeilingGenerator(range_ub=rng, divider=3),
            f"span_uncorrelated_{rng}": SpannerGenerator(range_ub=rng, base_generator=UncorrleatedGenerator, num_spanners=2, range_mult=10),
            f"span_weakly_correlated_{rng}": SpannerGenerator(range_ub=rng, base_generator=WeaklyCorrelatedGenerator, num_spanners=2, range_mult=10),
            f"span_strongly_correlated_{rng}": SpannerGenerator(range_ub=rng, base_generator=StronglyCorrelatedGenerator, num_spanners=2, range_mult=10),
            f"strongly_correlated_{rng}": StronglyCorrelatedGenerator(range_ub=rng),
            f"subset_sum_{rng}": SubsetSumGenerator(range_ub=rng),
            f"uncorrelated_{rng}": UncorrleatedGenerator(range_ub=rng),
            f"weakly_correlated_{rng}": WeaklyCorrelatedGenerator(range_ub=rng)
        }

    class_assigners = {
        "rnduniform": AssignClassesRandomUniform
    }

    resource_generators = {
        "rnduniform": GenerateResourcesRandomUniform
    }

    possible_num_items = (50, 100, 200, 500, 1000, 2000)

    possible_num_classes = (20, 100, 500)

    for num_items in possible_num_items:
        for num_classes in possible_num_classes:
            if num_classes >= num_items:
                continue

            for kpg_name, kpg in kp_generators.items():
                for assigner_name, assigner in class_assigners.items():
                    for generator_name, res_generator in resource_generators.items():
                        name = f"{kpg_name}-{assigner_name}-{generator_name}-{num_items}-{num_classes}"

                        print(f"Generating instances of type: {name}")
                        
                        class Generator(assigner, res_generator, KFKGenerator):
                            pass

                        generator = Generator(kp_generator=kpg, num_items=num_items, num_classes=num_classes)
                        instances = generator.generate_all(instance_tot=100)

                        for idx, instance in enumerate(instances):
                            basename = f"{name}-{idx}"
                            filename = realpath(f"../instances/{basename}.txt")
                            instance.to_file(filename=filename)
