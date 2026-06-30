from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


class IndependentRuntimeLayoutTest(unittest.TestCase):
    def test_r2_runtime_sources_exist(self):
        expected_files = [
            "src/r2_behavior_server.cpp",
            "src/r2_behavior_client.cpp",
            "include/r2_behavior/r2_behavior_server.hpp",
            "include/r2_behavior/r2_behavior_client.hpp",
            "plugins/condition/is_manual_start.cpp",
            "include/r2_behavior/plugins/condition/is_manual_start.hpp",
            "plugins/action/pub_nav2_goal.cpp",
            "include/r2_behavior/plugins/action/pub_nav2_goal.hpp",
            "plugins/action/pub_twist.cpp",
            "include/r2_behavior/plugins/action/pub_twist.hpp",
            "include/r2_behavior/custom_types.hpp",
        ]

        missing = [path for path in expected_files if not (ROOT / path).is_file()]
        self.assertEqual([], missing)

    def test_runtime_config_does_not_reference_pb2025_package(self):
        checked_files = [
            "package.xml",
            "launch/r2_behavior_launch.py",
            "params/r2_behavior.yaml",
            "params/r2_behavior_dry_run.yaml",
        ]

        offenders = [path for path in checked_files if "pb2025_sentry_behavior" in read(path)]
        self.assertEqual([], offenders)


if __name__ == "__main__":
    unittest.main()
