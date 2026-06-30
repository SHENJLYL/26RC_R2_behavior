import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import GroupAction
from launch.actions import SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.actions import PushRosNamespace
from launch_ros.actions import SetRemap
from launch_ros.descriptions import ParameterFile
from nav2_common.launch import RewrittenYaml


def generate_launch_description():
    bringup_dir = get_package_share_directory("r2_behavior")

    namespace = LaunchConfiguration("namespace")
    use_sim_time = LaunchConfiguration("use_sim_time")
    params_file = LaunchConfiguration("params_file")
    log_level = LaunchConfiguration("log_level")

    configured_params = ParameterFile(
        RewrittenYaml(
            source_file=params_file,
            root_key=namespace,
            param_rewrites={"use_sim_time": use_sim_time},
            convert_types=True,
        ),
        allow_substs=True,
    )

    return LaunchDescription(
        [
            SetEnvironmentVariable("RCUTILS_LOGGING_BUFFERED_STREAM", "1"),
            SetEnvironmentVariable("RCUTILS_COLORIZED_OUTPUT", "1"),
            DeclareLaunchArgument("namespace", default_value="", description="Top-level namespace"),
            DeclareLaunchArgument(
                "use_sim_time",
                default_value="false",
                description="Use simulation clock if true",
            ),
            DeclareLaunchArgument(
                "params_file",
                default_value=os.path.join(bringup_dir, "params", "r2_behavior.yaml"),
                description="Full path to the R2 behavior parameter file",
            ),
            DeclareLaunchArgument("log_level", default_value="info", description="ROS log level"),
            GroupAction(
                [
                    PushRosNamespace(namespace=namespace),
                    SetRemap("/tf", "tf"),
                    SetRemap("/tf_static", "tf_static"),
                    Node(
                        package="pb2025_sentry_behavior",
                        executable="pb2025_sentry_behavior_server",
                        name="pb2025_sentry_behavior_server",
                        output="screen",
                        parameters=[configured_params],
                        arguments=["--ros-args", "--log-level", log_level],
                    ),
                    Node(
                        package="pb2025_sentry_behavior",
                        executable="pb2025_sentry_behavior_client",
                        name="pb2025_sentry_behavior_client",
                        output="screen",
                        parameters=[configured_params],
                        arguments=["--ros-args", "--log-level", log_level],
                    ),
                ]
            ),
        ]
    )
