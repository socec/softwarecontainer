
# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE

import pytest

import subprocess
import os
import signal

from testframework import SoftwareContainerAgentHandler


# Add this directory (the package root) to the sys.path so the imports
# in the tests work, e.g. importing from 'testframework' to access helper
# classes etc.
package_root = os.path.dirname(os.path.abspath(__file__))
os.sys.path.insert(0, package_root)


@pytest.fixture(scope="module")
def dbus_launch():
    """ Setting up dbus environement variables for session bus """
    p = subprocess.Popen('dbus-launch', stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for var in p.stdout:
        sp = var.split('=', 1)
        env_var = sp[0]
        env_val = sp[1][:-1]
        os.environ[env_var] = env_val
    yield
    # The DBUS_SESSION_BUS_PID is set to sp[1] in the last loop iteration above...
    os.kill(int(sp[1]), signal.SIGKILL)


@pytest.fixture(scope="module")
def agent(request):
    """ Start the Agent and return an interface to it.

        This fixture requires the test module to have a function named 'logfile_path'
        on the module level
    """
    # Introspect the consuming module for the logfile path. The scope of this
    # fixture means the module is received through the 'request' parameter
    # and thus the path should be defined on the module level.
    logfile_path = request.module.logfile_path()
    agent_handler = SoftwareContainerAgentHandler(logfile_path)

    # Return the setup agent to the consuming test
    yield agent_handler

    # Do fixture teardown
    agent_handler.terminate()


def grep_for_dbus_proxy():
    """ Helper for 'assert_no_proxy' """
    return os.system('ps -aux | grep dbus-proxy | grep -v "grep" | grep prefix-dbus- > /dev/null')


@pytest.fixture(scope="function")
def assert_no_proxy():
    """ Assert that dbus-proxy is not running when we expect it to not be.

        Do the check both on setup and teardown
    """
    assert grep_for_dbus_proxy() != 0, "dbus-proxy is alive when it shouln't be"
    yield
    assert grep_for_dbus_proxy() != 0, "dbus-proxy is alive when it shouln't be"
