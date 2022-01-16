set(name simple-network-example)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name network-server-example)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name complete-network-example)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name adr-example)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name energy-model-example)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name aloha-throughput)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name parallel-reception-example)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)

set(name frame-counter-update)
set(source_files ${name}.cc)
set(header_files)
set(libraries_to_link ${libcore} ${liblorawan})
build_lib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)
