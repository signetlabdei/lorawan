# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('lorawan', ['core', 'network', 'propagation', 'mobility', 'point-to-point', 'energy', 'buildings'])
    module.source = [
        'model/lora-net-device.cc',
        'model/lora-mac.cc',
        'model/lora-phy.cc',
        'model/building-penetration-loss.cc',
        'model/correlated-shadowing-propagation-loss-model.cc',
        'model/lora-channel.cc',
        'model/lora-interference-helper.cc',
        'model/gateway-lora-mac.cc',
        'model/end-device-lora-mac.cc',
        'model/gateway-lora-phy.cc',
        'model/end-device-lora-phy.cc',
        'model/simple-end-device-lora-phy.cc',
        'model/simple-gateway-lora-phy.cc',
        'model/sub-band.cc',
        'model/logical-lora-channel.cc',
        'model/logical-lora-channel-helper.cc',
        'model/periodic-sender.cc',
        'model/one-shot-sender.cc',
        'model/forwarder.cc',
        'model/lora-mac-header.cc',
        'model/lora-frame-header.cc',
        'model/mac-command.cc',
        'model/lora-device-address.cc',
        'model/lora-device-address-generator.cc',
        'model/lora-tag.cc',
        'model/simple-network-server.cc',
        'model/device-status.cc',
        'model/gateway-status.cc',
        'model/lora-radio-energy-model.cc',
        'model/lora-tx-current-model.cc',
        'model/lora-utils.cc',
        'helper/lora-radio-energy-model-helper.cc',
        'helper/lora-helper.cc',
        'helper/lora-phy-helper.cc',
        'helper/lora-mac-helper.cc',
        'helper/periodic-sender-helper.cc',
        'helper/one-shot-sender-helper.cc',
        'helper/forwarder-helper.cc',
        'helper/network-server-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('lorawan')
    module_test.source = [
        'test/lorawan-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'lorawan'
    headers.source = [
        'model/lora-net-device.h',
        'model/lora-mac.h',
        'model/lora-phy.h',
        'model/building-penetration-loss.h',
        'model/correlated-shadowing-propagation-loss-model.h',
        'model/lora-channel.h',
        'model/lora-interference-helper.h',
        'model/gateway-lora-mac.h',
        'model/end-device-lora-mac.h',
        'model/gateway-lora-phy.h',
        'model/end-device-lora-phy.h',
        'model/simple-end-device-lora-phy.h',
        'model/simple-gateway-lora-phy.h',
        'model/sub-band.h',
        'model/logical-lora-channel.h',
        'model/logical-lora-channel-helper.h',
        'model/periodic-sender.h',
        'model/one-shot-sender.h',
        'model/forwarder.h',
        'model/lora-mac-header.h',
        'model/lora-frame-header.h',
        'model/mac-command.h',
        'model/lora-device-address.h',
        'model/lora-device-address-generator.h',
        'model/lora-tag.h',
        'model/simple-network-server.h',
        'model/device-status.h',
        'model/gateway-status.h',
        'model/lora-radio-energy-model.h',
        'model/lora-tx-current-model.h',
        'model/lora-utils.h',
        'helper/lora-radio-energy-model-helper.h',
        'helper/lora-helper.h',
        'helper/lora-phy-helper.h',
        'helper/lora-mac-helper.h',
        'helper/periodic-sender-helper.h',
        'helper/one-shot-sender-helper.h',
        'helper/forwarder-helper.h',
        'helper/network-server-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # Comment to disable python bindings
    # bld.ns3_python_bindings()
