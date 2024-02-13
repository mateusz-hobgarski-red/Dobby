# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 Sky UK
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import test_utils
import subprocess
from time import sleep
from pathlib import Path

tests = [
    test_utils.Test("Pid limit default",
                    "sleepy",
                    "256",
                    "Starts container with no pid limit specified, checks if default pid limit is set for container"),
    # test_utils.Test("Pid limit no override",
    #                 "sleepy_pid_limit",
    #                 "1000",
    #                 "Starts container with pid limit specified in config.json, checks if that pid limit was not overriden"),
]

def start_dobby_daemon():
    """Starts DobbyDaemon service, this service is then run in background and must be closed with stop_dobby_daemon.

    Parameters:
    None

    Returns:
    subproc (subprocess.Popen): DobbyDaemon service process, True for platform xi_6

    """
    if test_utils.selected_platform == test_utils.Platforms.xi_6:
        subprocess.run(["systemctl", "stop", "dobby"])
    else:
        subprocess.run(["sudo", "pkill", "DobbyDaemon"])

    test_utils.print_log("Starting Dobby Daemon (logging to Journal)...", test_utils.Severity.debug)

    # as this process is running infinitely we cannot use run_command_line as it waits for execution to end
    subproc = subprocess.Popen(["sudo",
                                "DobbyDaemon",
                                "--nofork",
                                #"--noconsole",
                                #"--journald"
                                ],
                               universal_newlines=True,
                               bufsize = 0,# unbuffered stdout
                               stderr=subprocess.STDOUT, #stderr goes to stdout
                                 )
                               #stdout=subprocess.STDOUT,#PIPE,
                               #stderr=subprocess.STDOUT)#PIPE)
    sleep(0.5)
    return subproc


def stop_dobby_daemon():
    """Stops DobbyDaemon service. For Xi6 DobbyDaemon is running all the time so this function returns does nothing

    Parameters:
    None

    Returns:
    subproc (subprocess.run): killing DobbyDaemon process, True for platform xi_6

    """

    test_utils.print_log("Stopping Dobby Daemon", test_utils.Severity.debug)
    subproc = test_utils.run_command_line(["sudo", "pkill", "DobbyDaemon"])
    sleep(0.2)
    return subproc

def execute_test():
    subproc = start_dobby_daemon()
    output_table = []

    for test in tests:
        result = test_container(test.container_id, test.expected_output)
        output = test_utils.create_simple_test_output(test, result[0], result[1])

        output_table.append(output)
        test_utils.print_single_result(output)

    stop_dobby_daemon()

    return test_utils.count_print_results(output_table)


def test_container(container_id, expected_output):
    """Runs container and check if output contains expected output

    Parameters:
    container_id (string): name of container to run
    expected_output (string): output that should be provided by container

    Returns:
    (pass (bool), message (string)): Returns if expected output found and message

    """

    test_utils.print_log("Running %s container test" % container_id, test_utils.Severity.debug)

    with test_utils.untar_bundle(container_id) as bundle_path:
        command = ["DobbyTool",
                "start",
                container_id,
                bundle_path]

        status = test_utils.run_command_line(command)
        sleep(0.5)
        print("status.stdout: ", status.stdout)
        if "started '" + container_id + "' container" not in status.stdout:
            return False, "Container did not launch successfully"
        
        return validate_pid_limit(container_id, expected_output)


def validate_pid_limit(container_id, expected_output):
    """Helper function for finding if containers pid limit is as expected

    Parameters:
    container_id (string): name of container to run
    expected_output (string): expected containers pid limit

    Returns:
    (pass (bool), message (string)): True if pid limit matches expected_output

    """

    print("validate pid limit for '%s" % container_id)
    pid_limit = 0

    # check pids.max present in containers pid cgroup
    path = Path("/sys/fs/cgroup/pids/" + container_id + "/pids.max")
    if not path.is_file():
        return False, "%s not found" % path.absolute()

    with open(path, 'r') as fh:
        pid_limit = fh.readline().strip()

    if expected_output == pid_limit:
        return True, "Test passed"
    else:
        return False, "Pid limit different then expected (expected: '%s', actual: '%s')" % (expected_output, pid_limit)


if __name__ == "__main__":
    test_utils.parse_arguments(__file__, True)
    execute_test()
