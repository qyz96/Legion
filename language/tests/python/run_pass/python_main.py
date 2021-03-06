#!/usr/bin/env python

# Copyright 2019 Stanford University
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from __future__ import print_function

from collections import OrderedDict
import legion
from legion import task, Fspace, R, Region, RW

# FIXME: Need a better way to determine task IDs.
hello = legion.extern_task(
    task_id=10000,
    argument_types=[legion.int64, legion.float64],
    return_type=legion.int64,
    calling_convention='regent')

saxpy = legion.extern_task(
    task_id=10001,
    argument_types=[Region, legion.float64],
    privileges=[RW],
    calling_convention='regent')

@task(privileges=[R])
def check(r):
    assert (r.x == 3.5).all()
    print('results validated successfully')

# This task needs an explicit ID so that Regent knows what to call.
@task(task_id=2)
def main():
    print('hello from Python')
    x = hello(1234, 3.14)
    print('Python got result from Regent task: %s' % x.get())

    print('creating a field space with two fields')
    # Note: Need to use OrderedDict so that the field ordering matches Regent.
    fs = Fspace(OrderedDict([('x', legion.float64), ('y', legion.float64)]))

    print('creating a region with 12 elements')
    r = Region([12], fs)

    legion.fill(r, 'x', 1)
    legion.fill(r, 'y', 2)

    a = 1.5

    print('calling SAXPY task in Regent')
    saxpy(r, a)

    check(r)
