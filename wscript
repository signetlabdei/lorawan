# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('lorawan', ['core', 'network',
                                               'propagation', 'mobility',
                                               'point-to-point', 'energy',
                                               'buildings'])
    module.source = [
        'model/lora-net-device.cc',
        'model/lorawan-mac.cc',
        'model/lora-phy.cc',
        'model/building-penetration-loss.cc',
        'model/correlated-shadowing-propagation-loss-model.cc',
        'model/lora-channel.cc',
        'model/lora-interference-helper.cc',
        'model/gateway-lorawan-mac.cc',
        'model/end-device-lorawan-mac.cc',
        'model/class-a-end-device-lorawan-mac.cc',
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
        'model/lorawan-mac-header.cc',
        'model/lora-frame-header.cc',
        'model/mac-command.cc',
        'model/lora-device-address.cc',
        'model/lora-device-address-generator.cc',
        'model/lora-tag.cc',
        'model/network-server.cc',
        'model/network-status.cc',
        'model/network-controller.cc',
        'model/network-controller-components.cc',
        'model/network-scheduler.cc',
        'model/end-device-status.cc',
        'model/gateway-status.cc',
        'model/lora-radio-energy-model.cc',
        'model/lora-tx-current-model.cc',
        'model/lora-utils.cc',
        'model/adr-component.cc',
        'model/hex-grid-position-allocator.cc',
        'helper/lora-radio-energy-model-helper.cc',
        'helper/lora-helper.cc',
        'helper/lora-phy-helper.cc',
        'helper/lorawan-mac-helper.cc',
        'helper/periodic-sender-helper.cc',
        'helper/one-shot-sender-helper.cc',
        'helper/forwarder-helper.cc',
        'helper/network-server-helper.cc',
        'helper/lora-packet-tracker.cc',
        'test/utilities.cc',
        ]

    module_test = bld.create_ns3_module_test_library('lorawan')
    module_test.source = [
        'test/lorawan-test-suite.cc',
        'test/network-status-test-suite.cc',
        'test/network-scheduler-test-suite.cc',
        'test/network-server-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'lorawan'
    headers.source = [
        'model/lora-net-device.h',
        'model/lorawan-mac.h',
        'model/lora-phy.h',
        'model/building-penetration-loss.h',
        'model/correlated-shadowing-propagation-loss-model.h',
        'model/lora-channel.h',
        'model/lora-interference-helper.h',
        'model/gateway-lorawan-mac.h',
        'model/end-device-lorawan-mac.h',
        'model/class-a-end-device-lorawan-mac.h',
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
        'model/lorawan-mac-header.h',
        'model/lora-frame-header.h',
        'model/mac-command.h',
        'model/lora-device-address.h',
        'model/lora-device-address-generator.h',
        'model/lora-tag.h',
        'model/network-server.h',
        'model/network-status.h',
        'model/network-controller.h',
        'model/network-controller-components.h',
        'model/network-scheduler.h',
        'model/end-device-status.h',
        'model/gateway-status.h',
        'model/lora-radio-energy-model.h',
        'model/lora-tx-current-model.h',
        'model/lora-utils.h',
        'model/adr-component.h',
        'model/hex-grid-position-allocator.h',
        'helper/lora-radio-energy-model-helper.h',
        'helper/lora-helper.h',
        'helper/lora-phy-helper.h',
        'helper/lorawan-mac-helper.h',
        'helper/periodic-sender-helper.h',
        'helper/one-shot-sender-helper.h',
        'helper/forwarder-helper.h',
        'helper/network-server-helper.h',
        'helper/lora-packet-tracker.h',
        'test/utilities.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # Comment to disable python bindings
    # bld.ns3_python_bindings()
