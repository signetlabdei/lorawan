#! /usr/bin/env python3

# A list of C++ examples to run in order to ensure that they remain
# buildable and runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run, do_valgrind_run).
#
# See test.py for more information.
cpp_examples = [
    ("simple-network-example", "True", "True"),
    ("network-server-example", "True", "True"),
    ("complete-network-example", "True", "True"),
    ("adr-example", "True", "True"),
    ("lorawan-energy-model-example", "True", "True"),
    ("aloha-throughput", "True", "True"),
    ("parallel-reception-example", "True", "True"),
    ("frame-counter-update", "True", "True"),
]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
