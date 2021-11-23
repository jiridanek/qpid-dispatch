#!/usr/bin/env python3

#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License
#

import asyncio
import re
import sys
from typing import Tuple


async def start_dispatch(build: str) -> Tuple[asyncio.subprocess.Process, int]:
    p = await asyncio.subprocess.create_subprocess_shell(
        f"{build} -c ./qdrouterd.conf",
        stderr=asyncio.subprocess.PIPE)

    while True:
        line = (await p.stderr.readline()).decode()
        print('router says:', line, end='')
        m = re.search(r'SERVER \(notice\) Listening for HTTP on localhost:(\d\d+) \(console\)', line)
        if m:
            port = int(m.group(1))
            break

    return p, port


async def print_output(stream: asyncio.StreamReader):
    while True:
        line = await stream.readline()
        if line == b'':
            return
        print("router says:", line.decode(), end="")


async def main() -> int:
    dispatch_build = sys.argv[1]
    dispatch, console_port = await start_dispatch(dispatch_build)
    print("router finished starting up, console listening at port", console_port)
    printer = print_output(dispatch.stderr)
    tests = await asyncio.subprocess.create_subprocess_shell("yarn run playwright test", env={
        'baseURL': f'http://localhost:{console_port}',
    })
    dispatch.terminate()
    await printer
    return (await dispatch.wait()) + (await tests.wait())


if __name__ == '__main__':
    sys.exit(asyncio.run(main()))