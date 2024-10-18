set(base_examples
    simple-network-example
    network-server-example
    complete-network-example
    adr-example
    lorawan-energy-model-example
    aloha-throughput
    parallel-reception-example
    frame-counter-update
)

foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK ${liblorawan}
  )
endforeach()
