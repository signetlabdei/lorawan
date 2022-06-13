build_lib_example(
  NAME simple-network-example
  SOURCE_FILES simple-network-example.cc
  LIBRARIES_TO_LINK
  ${libcore} ${liblorawan}
)

build_lib_example(
  NAME network-server-example
  SOURCE_FILES network-server-example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME complete-network-example
  SOURCE_FILES complete-network-example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME adr-example
  SOURCE_FILES adr-example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME lorawan-energy-model-example
  SOURCE_FILES lorawan-energy-model-example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME aloha-throughput
  SOURCE_FILES aloha-throughput.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME parallel-reception-example
  SOURCE_FILES parallel-reception-example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME frame-counter-update
  SOURCE_FILES frame-counter-update.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME pcap-example
  SOURCE_FILES pcap-example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)