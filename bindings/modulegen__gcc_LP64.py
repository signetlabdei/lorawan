from pybindgen import Module, FileCodeSink, param, retval, cppclass, typehandlers


import pybindgen.settings
import warnings

class ErrorHandler(pybindgen.settings.ErrorHandler):
    def handle_error(self, wrapper, exception, traceback_):
        warnings.warn("exception %r in wrapper %s" % (exception, wrapper))
        return True
pybindgen.settings.error_handler = ErrorHandler()


import sys

def module_init():
    root_module = Module('ns.lorawan', cpp_namespace='::ns3')
    return root_module

def register_types(module):
    root_module = module.get_root()
    
    ## mac-command.h (module 'lorawan'): ns3::MacCommandType [enumeration]
    module.add_enum('MacCommandType', ['INVALID', 'LINK_CHECK_REQ', 'LINK_CHECK_ANS', 'LINK_ADR_REQ', 'LINK_ADR_ANS', 'DUTY_CYCLE_REQ', 'DUTY_CYCLE_ANS', 'RX_PARAM_SETUP_REQ', 'RX_PARAM_SETUP_ANS', 'DEV_STATUS_REQ', 'DEV_STATUS_ANS', 'NEW_CHANNEL_REQ', 'NEW_CHANNEL_ANS', 'RX_TIMING_SETUP_REQ', 'RX_TIMING_SETUP_ANS', 'TX_PARAM_SETUP_REQ', 'TX_PARAM_SETUP_ANS', 'DL_CHANNEL_REQ', 'DL_CHANNEL_ANS'])
    ## address.h (module 'network'): ns3::Address [class]
    module.add_class('Address', import_from_module='ns.network')
    ## address.h (module 'network'): ns3::Address::MaxSize_e [enumeration]
    module.add_enum('MaxSize_e', ['MAX_SIZE'], outer_class=root_module['ns3::Address'], import_from_module='ns.network')
    ## application-container.h (module 'network'): ns3::ApplicationContainer [class]
    module.add_class('ApplicationContainer', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelper [class]
    module.add_class('AsciiTraceHelper', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelperForDevice [class]
    module.add_class('AsciiTraceHelperForDevice', allow_subclassing=True, import_from_module='ns.network')
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList [class]
    module.add_class('AttributeConstructionList', import_from_module='ns.core')
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item [struct]
    module.add_class('Item', import_from_module='ns.core', outer_class=root_module['ns3::AttributeConstructionList'])
    ## buffer.h (module 'network'): ns3::Buffer [class]
    module.add_class('Buffer', import_from_module='ns.network')
    ## buffer.h (module 'network'): ns3::Buffer::Iterator [class]
    module.add_class('Iterator', import_from_module='ns.network', outer_class=root_module['ns3::Buffer'])
    ## packet.h (module 'network'): ns3::ByteTagIterator [class]
    module.add_class('ByteTagIterator', import_from_module='ns.network')
    ## packet.h (module 'network'): ns3::ByteTagIterator::Item [class]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::ByteTagIterator'])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList [class]
    module.add_class('ByteTagList', import_from_module='ns.network')
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator [class]
    module.add_class('Iterator', import_from_module='ns.network', outer_class=root_module['ns3::ByteTagList'])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item [struct]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::ByteTagList::Iterator'])
    ## callback.h (module 'core'): ns3::CallbackBase [class]
    module.add_class('CallbackBase', import_from_module='ns.core')
    ## data-rate.h (module 'network'): ns3::DataRate [class]
    module.add_class('DataRate', import_from_module='ns.network')
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeAccessor> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::AttributeAccessor'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeChecker> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::AttributeChecker'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeValue> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::AttributeValue'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::CallbackImplBase> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::CallbackImplBase'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::EventImpl> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::EventImpl'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Hash::Implementation> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::Hash::Implementation'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event> [struct]
    module.add_class('DefaultDeleter', template_parameters=['ns3::LoraInterferenceHelper::Event'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::NixVector> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::NixVector'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::OutputStreamWrapper> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::OutputStreamWrapper'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Packet> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::Packet'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::TraceSourceAccessor> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::TraceSourceAccessor'])
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus [class]
    module.add_class('DeviceStatus')
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply [struct]
    module.add_class('Reply', outer_class=root_module['ns3::DeviceStatus'])
    ## event-id.h (module 'core'): ns3::EventId [class]
    module.add_class('EventId', import_from_module='ns.core')
    ## forwarder-helper.h (module 'lorawan'): ns3::ForwarderHelper [class]
    module.add_class('ForwarderHelper')
    ## gateway-status.h (module 'lorawan'): ns3::GatewayStatus [class]
    module.add_class('GatewayStatus')
    ## hash.h (module 'core'): ns3::Hasher [class]
    module.add_class('Hasher', import_from_module='ns.core')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address [class]
    module.add_class('Ipv4Address', import_from_module='ns.network')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address [class]
    root_module['ns3::Ipv4Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Mask [class]
    module.add_class('Ipv4Mask', import_from_module='ns.network')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address [class]
    module.add_class('Ipv6Address', import_from_module='ns.network')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address [class]
    root_module['ns3::Ipv6Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix [class]
    module.add_class('Ipv6Prefix', import_from_module='ns.network')
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters [struct]
    module.add_class('LoraChannelParameters')
    ## lora-device-address.h (module 'lorawan'): ns3::LoraDeviceAddress [class]
    module.add_class('LoraDeviceAddress')
    ## lora-helper.h (module 'lorawan'): ns3::LoraHelper [class]
    module.add_class('LoraHelper', allow_subclassing=True)
    ## lora-interference-helper.h (module 'lorawan'): ns3::LoraInterferenceHelper [class]
    module.add_class('LoraInterferenceHelper')
    ## lora-mac-helper.h (module 'lorawan'): ns3::LoraMacHelper [class]
    module.add_class('LoraMacHelper')
    ## lora-mac-helper.h (module 'lorawan'): ns3::LoraMacHelper::DeviceType [enumeration]
    module.add_enum('DeviceType', ['GW', 'ED'], outer_class=root_module['ns3::LoraMacHelper'])
    ## lora-mac-helper.h (module 'lorawan'): ns3::LoraMacHelper::Regions [enumeration]
    module.add_enum('Regions', ['EU', 'US', 'China', 'EU433MHz', 'Australia', 'CN', 'AS923MHz', 'SouthKorea'], outer_class=root_module['ns3::LoraMacHelper'])
    ## lora-phy-helper.h (module 'lorawan'): ns3::LoraPhyHelper [class]
    module.add_class('LoraPhyHelper')
    ## lora-phy-helper.h (module 'lorawan'): ns3::LoraPhyHelper::DeviceType [enumeration]
    module.add_enum('DeviceType', ['GW', 'ED'], outer_class=root_module['ns3::LoraPhyHelper'])
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters [struct]
    module.add_class('LoraTxParameters')
    ## mac48-address.h (module 'network'): ns3::Mac48Address [class]
    module.add_class('Mac48Address', import_from_module='ns.network')
    ## mac48-address.h (module 'network'): ns3::Mac48Address [class]
    root_module['ns3::Mac48Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer [class]
    module.add_class('NetDeviceContainer', import_from_module='ns.network')
    ## network-server-helper.h (module 'lorawan'): ns3::NetworkServerHelper [class]
    module.add_class('NetworkServerHelper')
    ## node-container.h (module 'network'): ns3::NodeContainer [class]
    module.add_class('NodeContainer', import_from_module='ns.network')
    ## lora-device-address.h (module 'lorawan'): ns3::NwkAddr [class]
    module.add_class('NwkAddr')
    ## lora-device-address.h (module 'lorawan'): ns3::NwkID [class]
    module.add_class('NwkID')
    ## object-base.h (module 'core'): ns3::ObjectBase [class]
    module.add_class('ObjectBase', allow_subclassing=True, import_from_module='ns.core')
    ## object.h (module 'core'): ns3::ObjectDeleter [struct]
    module.add_class('ObjectDeleter', import_from_module='ns.core')
    ## object-factory.h (module 'core'): ns3::ObjectFactory [class]
    module.add_class('ObjectFactory', import_from_module='ns.core')
    ## one-shot-sender-helper.h (module 'lorawan'): ns3::OneShotSenderHelper [class]
    module.add_class('OneShotSenderHelper')
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata [class]
    module.add_class('PacketMetadata', import_from_module='ns.network')
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item [struct]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::PacketMetadata'])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::ItemType [enumeration]
    module.add_enum('ItemType', ['PAYLOAD', 'HEADER', 'TRAILER'], outer_class=root_module['ns3::PacketMetadata::Item'], import_from_module='ns.network')
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::ItemIterator [class]
    module.add_class('ItemIterator', import_from_module='ns.network', outer_class=root_module['ns3::PacketMetadata'])
    ## packet.h (module 'network'): ns3::PacketTagIterator [class]
    module.add_class('PacketTagIterator', import_from_module='ns.network')
    ## packet.h (module 'network'): ns3::PacketTagIterator::Item [class]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::PacketTagIterator'])
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList [class]
    module.add_class('PacketTagList', import_from_module='ns.network')
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData [struct]
    module.add_class('TagData', import_from_module='ns.network', outer_class=root_module['ns3::PacketTagList'])
    ## pcap-file.h (module 'network'): ns3::PcapFile [class]
    module.add_class('PcapFile', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::PcapHelper [class]
    module.add_class('PcapHelper', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::PcapHelper::DataLinkType [enumeration]
    module.add_enum('DataLinkType', ['DLT_NULL', 'DLT_EN10MB', 'DLT_PPP', 'DLT_RAW', 'DLT_IEEE802_11', 'DLT_LINUX_SLL', 'DLT_PRISM_HEADER', 'DLT_IEEE802_11_RADIO', 'DLT_IEEE802_15_4', 'DLT_NETLINK'], outer_class=root_module['ns3::PcapHelper'], import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::PcapHelperForDevice [class]
    module.add_class('PcapHelperForDevice', allow_subclassing=True, import_from_module='ns.network')
    ## periodic-sender-helper.h (module 'lorawan'): ns3::PeriodicSenderHelper [class]
    module.add_class('PeriodicSenderHelper')
    ## point-to-point-helper.h (module 'point-to-point'): ns3::PointToPointHelper [class]
    module.add_class('PointToPointHelper', import_from_module='ns.point_to_point', parent=[root_module['ns3::PcapHelperForDevice'], root_module['ns3::AsciiTraceHelperForDevice']])
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Object, ns3::ObjectBase, ns3::ObjectDeleter> [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Object', 'ns3::ObjectBase', 'ns3::ObjectDeleter'], parent=root_module['ns3::ObjectBase'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simulator.h (module 'core'): ns3::Simulator [class]
    module.add_class('Simulator', destructor_visibility='private', import_from_module='ns.core')
    ## simulator.h (module 'core'): ns3::Simulator [enumeration]
    module.add_enum('', ['NO_CONTEXT'], outer_class=root_module['ns3::Simulator'], import_from_module='ns.core')
    ## tag.h (module 'network'): ns3::Tag [class]
    module.add_class('Tag', import_from_module='ns.network', parent=root_module['ns3::ObjectBase'])
    ## tag-buffer.h (module 'network'): ns3::TagBuffer [class]
    module.add_class('TagBuffer', import_from_module='ns.network')
    ## nstime.h (module 'core'): ns3::TimeWithUnit [class]
    module.add_class('TimeWithUnit', import_from_module='ns.core')
    ## traced-value.h (module 'core'): ns3::TracedValue<double> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['double'])
    ## traced-value.h (module 'core'): ns3::TracedValue<int> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['int'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::EndDeviceLoraPhy::State> [class]
    module.add_class('TracedValue', template_parameters=['ns3::EndDeviceLoraPhy::State'])
    ## traced-value.h (module 'core'): ns3::TracedValue<unsigned char> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['unsigned char'])
    ## type-id.h (module 'core'): ns3::TypeId [class]
    module.add_class('TypeId', import_from_module='ns.core')
    ## type-id.h (module 'core'): ns3::TypeId::AttributeFlag [enumeration]
    module.add_enum('AttributeFlag', ['ATTR_GET', 'ATTR_SET', 'ATTR_CONSTRUCT', 'ATTR_SGC'], outer_class=root_module['ns3::TypeId'], import_from_module='ns.core')
    ## type-id.h (module 'core'): ns3::TypeId::SupportLevel [enumeration]
    module.add_enum('SupportLevel', ['SUPPORTED', 'DEPRECATED', 'OBSOLETE'], outer_class=root_module['ns3::TypeId'], import_from_module='ns.core')
    ## type-id.h (module 'core'): ns3::TypeId::AttributeInformation [struct]
    module.add_class('AttributeInformation', import_from_module='ns.core', outer_class=root_module['ns3::TypeId'])
    ## type-id.h (module 'core'): ns3::TypeId::TraceSourceInformation [struct]
    module.add_class('TraceSourceInformation', import_from_module='ns.core', outer_class=root_module['ns3::TypeId'])
    ## vector.h (module 'core'): ns3::Vector2D [class]
    module.add_class('Vector2D', import_from_module='ns.core')
    ## vector.h (module 'core'): ns3::Vector3D [class]
    module.add_class('Vector3D', import_from_module='ns.core')
    ## empty.h (module 'core'): ns3::empty [class]
    module.add_class('empty', import_from_module='ns.core')
    ## int64x64-128.h (module 'core'): ns3::int64x64_t [class]
    module.add_class('int64x64_t', import_from_module='ns.core')
    ## int64x64-128.h (module 'core'): ns3::int64x64_t::impl_type [enumeration]
    module.add_enum('impl_type', ['int128_impl', 'cairo_impl', 'ld_impl'], outer_class=root_module['ns3::int64x64_t'], import_from_module='ns.core')
    ## chunk.h (module 'network'): ns3::Chunk [class]
    module.add_class('Chunk', import_from_module='ns.network', parent=root_module['ns3::ObjectBase'])
    ## header.h (module 'network'): ns3::Header [class]
    module.add_class('Header', import_from_module='ns.network', parent=root_module['ns3::Chunk'])
    ## lora-frame-header.h (module 'lorawan'): ns3::LoraFrameHeader [class]
    module.add_class('LoraFrameHeader', parent=root_module['ns3::Header'])
    ## lora-mac-header.h (module 'lorawan'): ns3::LoraMacHeader [class]
    module.add_class('LoraMacHeader', parent=root_module['ns3::Header'])
    ## lora-mac-header.h (module 'lorawan'): ns3::LoraMacHeader::MType [enumeration]
    module.add_enum('MType', ['JOIN_REQUEST', 'JOIN_ACCEPT', 'UNCONFIRMED_DATA_UP', 'UNCONFIRMED_DATA_DOWN', 'CONFIRMED_DATA_UP', 'CONFIRMED_DATA_DOWN', 'PROPRIETARY'], outer_class=root_module['ns3::LoraMacHeader'])
    ## lora-tag.h (module 'lorawan'): ns3::LoraTag [class]
    module.add_class('LoraTag', parent=root_module['ns3::Tag'])
    ## object.h (module 'core'): ns3::Object [class]
    module.add_class('Object', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::Object, ns3::ObjectBase, ns3::ObjectDeleter >'])
    ## object.h (module 'core'): ns3::Object::AggregateIterator [class]
    module.add_class('AggregateIterator', import_from_module='ns.core', outer_class=root_module['ns3::Object'])
    ## pcap-file-wrapper.h (module 'network'): ns3::PcapFileWrapper [class]
    module.add_class('PcapFileWrapper', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## propagation-delay-model.h (module 'propagation'): ns3::PropagationDelayModel [class]
    module.add_class('PropagationDelayModel', import_from_module='ns.propagation', parent=root_module['ns3::Object'])
    ## propagation-loss-model.h (module 'propagation'): ns3::PropagationLossModel [class]
    module.add_class('PropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::Object'])
    ## propagation-delay-model.h (module 'propagation'): ns3::RandomPropagationDelayModel [class]
    module.add_class('RandomPropagationDelayModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationDelayModel'])
    ## propagation-loss-model.h (module 'propagation'): ns3::RandomPropagationLossModel [class]
    module.add_class('RandomPropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## random-variable-stream.h (module 'core'): ns3::RandomVariableStream [class]
    module.add_class('RandomVariableStream', import_from_module='ns.core', parent=root_module['ns3::Object'])
    ## propagation-loss-model.h (module 'propagation'): ns3::RangePropagationLossModel [class]
    module.add_class('RangePropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## random-variable-stream.h (module 'core'): ns3::SequentialRandomVariable [class]
    module.add_class('SequentialRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::AttributeAccessor, ns3::empty, ns3::DefaultDeleter<ns3::AttributeAccessor> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::AttributeAccessor', 'ns3::empty', 'ns3::DefaultDeleter<ns3::AttributeAccessor>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::AttributeChecker, ns3::empty, ns3::DefaultDeleter<ns3::AttributeChecker> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::AttributeChecker', 'ns3::empty', 'ns3::DefaultDeleter<ns3::AttributeChecker>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::AttributeValue, ns3::empty, ns3::DefaultDeleter<ns3::AttributeValue> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::AttributeValue', 'ns3::empty', 'ns3::DefaultDeleter<ns3::AttributeValue>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::CallbackImplBase, ns3::empty, ns3::DefaultDeleter<ns3::CallbackImplBase> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::CallbackImplBase', 'ns3::empty', 'ns3::DefaultDeleter<ns3::CallbackImplBase>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::EventImpl, ns3::empty, ns3::DefaultDeleter<ns3::EventImpl> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::EventImpl', 'ns3::empty', 'ns3::DefaultDeleter<ns3::EventImpl>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Hash::Implementation, ns3::empty, ns3::DefaultDeleter<ns3::Hash::Implementation> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Hash::Implementation', 'ns3::empty', 'ns3::DefaultDeleter<ns3::Hash::Implementation>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::LoraInterferenceHelper::Event, ns3::empty, ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, template_parameters=['ns3::LoraInterferenceHelper::Event', 'ns3::empty', 'ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::NixVector, ns3::empty, ns3::DefaultDeleter<ns3::NixVector> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::NixVector', 'ns3::empty', 'ns3::DefaultDeleter<ns3::NixVector>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::OutputStreamWrapper, ns3::empty, ns3::DefaultDeleter<ns3::OutputStreamWrapper> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::OutputStreamWrapper', 'ns3::empty', 'ns3::DefaultDeleter<ns3::OutputStreamWrapper>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Packet, ns3::empty, ns3::DefaultDeleter<ns3::Packet> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Packet', 'ns3::empty', 'ns3::DefaultDeleter<ns3::Packet>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::TraceSourceAccessor, ns3::empty, ns3::DefaultDeleter<ns3::TraceSourceAccessor> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::TraceSourceAccessor', 'ns3::empty', 'ns3::DefaultDeleter<ns3::TraceSourceAccessor>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## sub-band.h (module 'lorawan'): ns3::SubBand [class]
    module.add_class('SubBand', parent=root_module['ns3::Object'])
    ## propagation-loss-model.h (module 'propagation'): ns3::ThreeLogDistancePropagationLossModel [class]
    module.add_class('ThreeLogDistancePropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## nstime.h (module 'core'): ns3::Time [class]
    module.add_class('Time', import_from_module='ns.core')
    ## nstime.h (module 'core'): ns3::Time::Unit [enumeration]
    module.add_enum('Unit', ['Y', 'D', 'H', 'MIN', 'S', 'MS', 'US', 'NS', 'PS', 'FS', 'LAST'], outer_class=root_module['ns3::Time'], import_from_module='ns.core')
    ## nstime.h (module 'core'): ns3::Time [class]
    root_module['ns3::Time'].implicitly_converts_to(root_module['ns3::int64x64_t'])
    ## trace-source-accessor.h (module 'core'): ns3::TraceSourceAccessor [class]
    module.add_class('TraceSourceAccessor', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::TraceSourceAccessor, ns3::empty, ns3::DefaultDeleter<ns3::TraceSourceAccessor> >'])
    ## trailer.h (module 'network'): ns3::Trailer [class]
    module.add_class('Trailer', import_from_module='ns.network', parent=root_module['ns3::Chunk'])
    ## random-variable-stream.h (module 'core'): ns3::TriangularRandomVariable [class]
    module.add_class('TriangularRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## propagation-loss-model.h (module 'propagation'): ns3::TwoRayGroundPropagationLossModel [class]
    module.add_class('TwoRayGroundPropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## random-variable-stream.h (module 'core'): ns3::UniformRandomVariable [class]
    module.add_class('UniformRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## random-variable-stream.h (module 'core'): ns3::WeibullRandomVariable [class]
    module.add_class('WeibullRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## random-variable-stream.h (module 'core'): ns3::ZetaRandomVariable [class]
    module.add_class('ZetaRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## random-variable-stream.h (module 'core'): ns3::ZipfRandomVariable [class]
    module.add_class('ZipfRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## application.h (module 'network'): ns3::Application [class]
    module.add_class('Application', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## attribute.h (module 'core'): ns3::AttributeAccessor [class]
    module.add_class('AttributeAccessor', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::AttributeAccessor, ns3::empty, ns3::DefaultDeleter<ns3::AttributeAccessor> >'])
    ## attribute.h (module 'core'): ns3::AttributeChecker [class]
    module.add_class('AttributeChecker', allow_subclassing=False, automatic_type_narrowing=True, import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::AttributeChecker, ns3::empty, ns3::DefaultDeleter<ns3::AttributeChecker> >'])
    ## attribute.h (module 'core'): ns3::AttributeValue [class]
    module.add_class('AttributeValue', allow_subclassing=False, automatic_type_narrowing=True, import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::AttributeValue, ns3::empty, ns3::DefaultDeleter<ns3::AttributeValue> >'])
    ## boolean.h (module 'core'): ns3::BooleanChecker [class]
    module.add_class('BooleanChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## boolean.h (module 'core'): ns3::BooleanValue [class]
    module.add_class('BooleanValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## callback.h (module 'core'): ns3::CallbackChecker [class]
    module.add_class('CallbackChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## callback.h (module 'core'): ns3::CallbackImplBase [class]
    module.add_class('CallbackImplBase', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::CallbackImplBase, ns3::empty, ns3::DefaultDeleter<ns3::CallbackImplBase> >'])
    ## callback.h (module 'core'): ns3::CallbackValue [class]
    module.add_class('CallbackValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## channel.h (module 'network'): ns3::Channel [class]
    module.add_class('Channel', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## random-variable-stream.h (module 'core'): ns3::ConstantRandomVariable [class]
    module.add_class('ConstantRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## propagation-delay-model.h (module 'propagation'): ns3::ConstantSpeedPropagationDelayModel [class]
    module.add_class('ConstantSpeedPropagationDelayModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationDelayModel'])
    ## data-rate.h (module 'network'): ns3::DataRateChecker [class]
    module.add_class('DataRateChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## data-rate.h (module 'network'): ns3::DataRateValue [class]
    module.add_class('DataRateValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## random-variable-stream.h (module 'core'): ns3::DeterministicRandomVariable [class]
    module.add_class('DeterministicRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## double.h (module 'core'): ns3::DoubleValue [class]
    module.add_class('DoubleValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## random-variable-stream.h (module 'core'): ns3::EmpiricalRandomVariable [class]
    module.add_class('EmpiricalRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## attribute.h (module 'core'): ns3::EmptyAttributeAccessor [class]
    module.add_class('EmptyAttributeAccessor', import_from_module='ns.core', parent=root_module['ns3::AttributeAccessor'])
    ## attribute.h (module 'core'): ns3::EmptyAttributeChecker [class]
    module.add_class('EmptyAttributeChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## attribute.h (module 'core'): ns3::EmptyAttributeValue [class]
    module.add_class('EmptyAttributeValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## enum.h (module 'core'): ns3::EnumChecker [class]
    module.add_class('EnumChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## enum.h (module 'core'): ns3::EnumValue [class]
    module.add_class('EnumValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## random-variable-stream.h (module 'core'): ns3::ErlangRandomVariable [class]
    module.add_class('ErlangRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## event-impl.h (module 'core'): ns3::EventImpl [class]
    module.add_class('EventImpl', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::EventImpl, ns3::empty, ns3::DefaultDeleter<ns3::EventImpl> >'])
    ## random-variable-stream.h (module 'core'): ns3::ExponentialRandomVariable [class]
    module.add_class('ExponentialRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## propagation-loss-model.h (module 'propagation'): ns3::FixedRssLossModel [class]
    module.add_class('FixedRssLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## forwarder.h (module 'lorawan'): ns3::Forwarder [class]
    module.add_class('Forwarder', parent=root_module['ns3::Application'])
    ## propagation-loss-model.h (module 'propagation'): ns3::FriisPropagationLossModel [class]
    module.add_class('FriisPropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## random-variable-stream.h (module 'core'): ns3::GammaRandomVariable [class]
    module.add_class('GammaRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## integer.h (module 'core'): ns3::IntegerValue [class]
    module.add_class('IntegerValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4AddressChecker [class]
    module.add_class('Ipv4AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4AddressValue [class]
    module.add_class('Ipv4AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4MaskChecker [class]
    module.add_class('Ipv4MaskChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4MaskValue [class]
    module.add_class('Ipv4MaskValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6AddressChecker [class]
    module.add_class('Ipv6AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6AddressValue [class]
    module.add_class('Ipv6AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6PrefixChecker [class]
    module.add_class('Ipv6PrefixChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6PrefixValue [class]
    module.add_class('Ipv6PrefixValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## propagation-loss-model.h (module 'propagation'): ns3::LogDistancePropagationLossModel [class]
    module.add_class('LogDistancePropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## random-variable-stream.h (module 'core'): ns3::LogNormalRandomVariable [class]
    module.add_class('LogNormalRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## logical-lora-channel.h (module 'lorawan'): ns3::LogicalLoraChannel [class]
    module.add_class('LogicalLoraChannel', parent=root_module['ns3::Object'])
    ## logical-lora-channel-helper.h (module 'lorawan'): ns3::LogicalLoraChannelHelper [class]
    module.add_class('LogicalLoraChannelHelper', parent=root_module['ns3::Object'])
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannel [class]
    module.add_class('LoraChannel', parent=root_module['ns3::Channel'])
    ## lora-device-address-generator.h (module 'lorawan'): ns3::LoraDeviceAddressGenerator [class]
    module.add_class('LoraDeviceAddressGenerator', parent=root_module['ns3::Object'])
    ## lora-mac.h (module 'lorawan'): ns3::LoraMac [class]
    module.add_class('LoraMac', parent=root_module['ns3::Object'])
    ## lora-phy.h (module 'lorawan'): ns3::LoraPhy [class]
    module.add_class('LoraPhy', parent=root_module['ns3::Object'])
    ## mac48-address.h (module 'network'): ns3::Mac48AddressChecker [class]
    module.add_class('Mac48AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## mac48-address.h (module 'network'): ns3::Mac48AddressValue [class]
    module.add_class('Mac48AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## mac-command.h (module 'lorawan'): ns3::MacCommand [class]
    module.add_class('MacCommand', parent=root_module['ns3::Object'])
    ## propagation-loss-model.h (module 'propagation'): ns3::MatrixPropagationLossModel [class]
    module.add_class('MatrixPropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## mobility-model.h (module 'mobility'): ns3::MobilityModel [class]
    module.add_class('MobilityModel', import_from_module='ns.mobility', parent=root_module['ns3::Object'])
    ## propagation-loss-model.h (module 'propagation'): ns3::NakagamiPropagationLossModel [class]
    module.add_class('NakagamiPropagationLossModel', import_from_module='ns.propagation', parent=root_module['ns3::PropagationLossModel'])
    ## net-device.h (module 'network'): ns3::NetDevice [class]
    module.add_class('NetDevice', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## net-device.h (module 'network'): ns3::NetDevice::PacketType [enumeration]
    module.add_enum('PacketType', ['PACKET_HOST', 'NS3_PACKET_HOST', 'PACKET_BROADCAST', 'NS3_PACKET_BROADCAST', 'PACKET_MULTICAST', 'NS3_PACKET_MULTICAST', 'PACKET_OTHERHOST', 'NS3_PACKET_OTHERHOST'], outer_class=root_module['ns3::NetDevice'], import_from_module='ns.network')
    ## mac-command.h (module 'lorawan'): ns3::NewChannelAns [class]
    module.add_class('NewChannelAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::NewChannelReq [class]
    module.add_class('NewChannelReq', parent=root_module['ns3::MacCommand'])
    ## nix-vector.h (module 'network'): ns3::NixVector [class]
    module.add_class('NixVector', import_from_module='ns.network', parent=root_module['ns3::SimpleRefCount< ns3::NixVector, ns3::empty, ns3::DefaultDeleter<ns3::NixVector> >'])
    ## node.h (module 'network'): ns3::Node [class]
    module.add_class('Node', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## random-variable-stream.h (module 'core'): ns3::NormalRandomVariable [class]
    module.add_class('NormalRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## object-factory.h (module 'core'): ns3::ObjectFactoryChecker [class]
    module.add_class('ObjectFactoryChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## object-factory.h (module 'core'): ns3::ObjectFactoryValue [class]
    module.add_class('ObjectFactoryValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## one-shot-sender.h (module 'lorawan'): ns3::OneShotSender [class]
    module.add_class('OneShotSender', parent=root_module['ns3::Application'])
    ## output-stream-wrapper.h (module 'network'): ns3::OutputStreamWrapper [class]
    module.add_class('OutputStreamWrapper', import_from_module='ns.network', parent=root_module['ns3::SimpleRefCount< ns3::OutputStreamWrapper, ns3::empty, ns3::DefaultDeleter<ns3::OutputStreamWrapper> >'])
    ## packet.h (module 'network'): ns3::Packet [class]
    module.add_class('Packet', import_from_module='ns.network', parent=root_module['ns3::SimpleRefCount< ns3::Packet, ns3::empty, ns3::DefaultDeleter<ns3::Packet> >'])
    ## random-variable-stream.h (module 'core'): ns3::ParetoRandomVariable [class]
    module.add_class('ParetoRandomVariable', import_from_module='ns.core', parent=root_module['ns3::RandomVariableStream'])
    ## periodic-sender.h (module 'lorawan'): ns3::PeriodicSender [class]
    module.add_class('PeriodicSender', parent=root_module['ns3::Application'])
    ## point-to-point-net-device.h (module 'point-to-point'): ns3::PointToPointNetDevice [class]
    module.add_class('PointToPointNetDevice', import_from_module='ns.point_to_point', parent=root_module['ns3::NetDevice'])
    ## mac-command.h (module 'lorawan'): ns3::RxParamSetupAns [class]
    module.add_class('RxParamSetupAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::RxParamSetupReq [class]
    module.add_class('RxParamSetupReq', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::RxTimingSetupAns [class]
    module.add_class('RxTimingSetupAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::RxTimingSetupReq [class]
    module.add_class('RxTimingSetupReq', parent=root_module['ns3::MacCommand'])
    ## simple-network-server.h (module 'lorawan'): ns3::SimpleNetworkServer [class]
    module.add_class('SimpleNetworkServer', parent=root_module['ns3::Application'])
    ## nstime.h (module 'core'): ns3::TimeValue [class]
    module.add_class('TimeValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## mac-command.h (module 'lorawan'): ns3::TxParamSetupAns [class]
    module.add_class('TxParamSetupAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::TxParamSetupReq [class]
    module.add_class('TxParamSetupReq', parent=root_module['ns3::MacCommand'])
    ## type-id.h (module 'core'): ns3::TypeIdChecker [class]
    module.add_class('TypeIdChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## type-id.h (module 'core'): ns3::TypeIdValue [class]
    module.add_class('TypeIdValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## uinteger.h (module 'core'): ns3::UintegerValue [class]
    module.add_class('UintegerValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## vector.h (module 'core'): ns3::Vector2DChecker [class]
    module.add_class('Vector2DChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## vector.h (module 'core'): ns3::Vector2DValue [class]
    module.add_class('Vector2DValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## vector.h (module 'core'): ns3::Vector3DChecker [class]
    module.add_class('Vector3DChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## vector.h (module 'core'): ns3::Vector3DValue [class]
    module.add_class('Vector3DValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## address.h (module 'network'): ns3::AddressChecker [class]
    module.add_class('AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## address.h (module 'network'): ns3::AddressValue [class]
    module.add_class('AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## callback.h (module 'core'): ns3::CallbackImpl<bool, ns3::Ptr<ns3::NetDevice>, ns3::Ptr<const ns3::Packet>, unsigned short, const ns3::Address &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['bool', 'ns3::Ptr<ns3::NetDevice>', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<ns3::ObjectBase *, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['ns3::ObjectBase *', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, double, double, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'double', 'double', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, int, int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'int', 'int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::EndDeviceLoraPhy::State, ns3::EndDeviceLoraPhy::State, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', template_parameters=['void', 'ns3::EndDeviceLoraPhy::State', 'ns3::EndDeviceLoraPhy::State', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::MobilityModel>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::MobilityModel>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::Packet>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::Packet>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::Packet>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::NetDevice>, ns3::Ptr<const ns3::Packet>, unsigned short, const ns3::Address &, const ns3::Address &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::NetDevice>', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'const ns3::Address &', 'const ns3::Address &', 'ns3::NetDevice::PacketType', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::NetDevice>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::NetDevice>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, unsigned char, unsigned char, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'unsigned char', 'unsigned char', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## mac-command.h (module 'lorawan'): ns3::DevStatusAns [class]
    module.add_class('DevStatusAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::DevStatusReq [class]
    module.add_class('DevStatusReq', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::DlChannelAns [class]
    module.add_class('DlChannelAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::DutyCycleAns [class]
    module.add_class('DutyCycleAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::DutyCycleReq [class]
    module.add_class('DutyCycleReq', parent=root_module['ns3::MacCommand'])
    ## end-device-lora-mac.h (module 'lorawan'): ns3::EndDeviceLoraMac [class]
    module.add_class('EndDeviceLoraMac', parent=root_module['ns3::LoraMac'])
    ## end-device-lora-phy.h (module 'lorawan'): ns3::EndDeviceLoraPhy [class]
    module.add_class('EndDeviceLoraPhy', parent=root_module['ns3::LoraPhy'])
    ## end-device-lora-phy.h (module 'lorawan'): ns3::EndDeviceLoraPhy::State [enumeration]
    module.add_enum('State', ['SLEEP', 'STANDBY', 'TX', 'RX'], outer_class=root_module['ns3::EndDeviceLoraPhy'])
    ## gateway-lora-mac.h (module 'lorawan'): ns3::GatewayLoraMac [class]
    module.add_class('GatewayLoraMac', parent=root_module['ns3::LoraMac'])
    ## gateway-lora-phy.h (module 'lorawan'): ns3::GatewayLoraPhy [class]
    module.add_class('GatewayLoraPhy', parent=root_module['ns3::LoraPhy'])
    ## mac-command.h (module 'lorawan'): ns3::LinkAdrAns [class]
    module.add_class('LinkAdrAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::LinkAdrReq [class]
    module.add_class('LinkAdrReq', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::LinkCheckAns [class]
    module.add_class('LinkCheckAns', parent=root_module['ns3::MacCommand'])
    ## mac-command.h (module 'lorawan'): ns3::LinkCheckReq [class]
    module.add_class('LinkCheckReq', parent=root_module['ns3::MacCommand'])
    ## lora-net-device.h (module 'lorawan'): ns3::LoraNetDevice [class]
    module.add_class('LoraNetDevice', parent=root_module['ns3::NetDevice'])
    module.add_container('std::list< ns3::Address >', 'ns3::Address', container_type=u'list')
    module.add_container('std::list< ns3::Ptr< ns3::LoraInterferenceHelper::Event > >', 'ns3::Ptr< ns3::LoraInterferenceHelper::Event >', container_type=u'list')
    module.add_container('std::list< int >', 'int', container_type=u'list')
    module.add_container('std::list< ns3::Ptr< ns3::MacCommand > >', 'ns3::Ptr< ns3::MacCommand >', container_type=u'list')
    module.add_container('std::vector< ns3::Ptr< ns3::LogicalLoraChannel > >', 'ns3::Ptr< ns3::LogicalLoraChannel >', container_type=u'vector')
    module.add_container('std::vector< unsigned char >', 'unsigned char', container_type=u'vector')
    module.add_container('std::vector< double >', 'double', container_type=u'vector')
    module.add_container('std::vector< unsigned int >', 'unsigned int', container_type=u'vector')
    typehandlers.add_type_alias(u'ns3::Vector3D', u'ns3::Vector')
    typehandlers.add_type_alias(u'ns3::Vector3D*', u'ns3::Vector*')
    typehandlers.add_type_alias(u'ns3::Vector3D&', u'ns3::Vector&')
    module.add_typedef(root_module['ns3::Vector3D'], 'Vector')
    typehandlers.add_type_alias(u'ns3::Vector3DValue', u'ns3::VectorValue')
    typehandlers.add_type_alias(u'ns3::Vector3DValue*', u'ns3::VectorValue*')
    typehandlers.add_type_alias(u'ns3::Vector3DValue&', u'ns3::VectorValue&')
    module.add_typedef(root_module['ns3::Vector3DValue'], 'VectorValue')
    typehandlers.add_type_alias(u'ns3::Vector3DChecker', u'ns3::VectorChecker')
    typehandlers.add_type_alias(u'ns3::Vector3DChecker*', u'ns3::VectorChecker*')
    typehandlers.add_type_alias(u'ns3::Vector3DChecker&', u'ns3::VectorChecker&')
    module.add_typedef(root_module['ns3::Vector3DChecker'], 'VectorChecker')
    
    ## Register a nested module for the namespace FatalImpl
    
    nested_module = module.add_cpp_namespace('FatalImpl')
    register_types_ns3_FatalImpl(nested_module)
    
    
    ## Register a nested module for the namespace Hash
    
    nested_module = module.add_cpp_namespace('Hash')
    register_types_ns3_Hash(nested_module)
    
    
    ## Register a nested module for the namespace TracedValueCallback
    
    nested_module = module.add_cpp_namespace('TracedValueCallback')
    register_types_ns3_TracedValueCallback(nested_module)
    
    
    ## Register a nested module for the namespace internal
    
    nested_module = module.add_cpp_namespace('internal')
    register_types_ns3_internal(nested_module)
    

def register_types_ns3_FatalImpl(module):
    root_module = module.get_root()
    

def register_types_ns3_Hash(module):
    root_module = module.get_root()
    
    ## hash-function.h (module 'core'): ns3::Hash::Implementation [class]
    module.add_class('Implementation', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::Hash::Implementation, ns3::empty, ns3::DefaultDeleter<ns3::Hash::Implementation> >'])
    typehandlers.add_type_alias(u'uint32_t ( * ) ( char const *, size_t const )', u'ns3::Hash::Hash32Function_ptr')
    typehandlers.add_type_alias(u'uint32_t ( * ) ( char const *, size_t const )*', u'ns3::Hash::Hash32Function_ptr*')
    typehandlers.add_type_alias(u'uint32_t ( * ) ( char const *, size_t const )&', u'ns3::Hash::Hash32Function_ptr&')
    typehandlers.add_type_alias(u'uint64_t ( * ) ( char const *, size_t const )', u'ns3::Hash::Hash64Function_ptr')
    typehandlers.add_type_alias(u'uint64_t ( * ) ( char const *, size_t const )*', u'ns3::Hash::Hash64Function_ptr*')
    typehandlers.add_type_alias(u'uint64_t ( * ) ( char const *, size_t const )&', u'ns3::Hash::Hash64Function_ptr&')
    
    ## Register a nested module for the namespace Function
    
    nested_module = module.add_cpp_namespace('Function')
    register_types_ns3_Hash_Function(nested_module)
    

def register_types_ns3_Hash_Function(module):
    root_module = module.get_root()
    
    ## hash-fnv.h (module 'core'): ns3::Hash::Function::Fnv1a [class]
    module.add_class('Fnv1a', import_from_module='ns.core', parent=root_module['ns3::Hash::Implementation'])
    ## hash-function.h (module 'core'): ns3::Hash::Function::Hash32 [class]
    module.add_class('Hash32', import_from_module='ns.core', parent=root_module['ns3::Hash::Implementation'])
    ## hash-function.h (module 'core'): ns3::Hash::Function::Hash64 [class]
    module.add_class('Hash64', import_from_module='ns.core', parent=root_module['ns3::Hash::Implementation'])
    ## hash-murmur3.h (module 'core'): ns3::Hash::Function::Murmur3 [class]
    module.add_class('Murmur3', import_from_module='ns.core', parent=root_module['ns3::Hash::Implementation'])

def register_types_ns3_TracedValueCallback(module):
    root_module = module.get_root()
    
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Time, ns3::Time )', u'ns3::TracedValueCallback::Time')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Time, ns3::Time )*', u'ns3::TracedValueCallback::Time*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Time, ns3::Time )&', u'ns3::TracedValueCallback::Time&')
    typehandlers.add_type_alias(u'void ( * ) ( bool, bool )', u'ns3::TracedValueCallback::Bool')
    typehandlers.add_type_alias(u'void ( * ) ( bool, bool )*', u'ns3::TracedValueCallback::Bool*')
    typehandlers.add_type_alias(u'void ( * ) ( bool, bool )&', u'ns3::TracedValueCallback::Bool&')
    typehandlers.add_type_alias(u'void ( * ) ( int8_t, int8_t )', u'ns3::TracedValueCallback::Int8')
    typehandlers.add_type_alias(u'void ( * ) ( int8_t, int8_t )*', u'ns3::TracedValueCallback::Int8*')
    typehandlers.add_type_alias(u'void ( * ) ( int8_t, int8_t )&', u'ns3::TracedValueCallback::Int8&')
    typehandlers.add_type_alias(u'void ( * ) ( uint8_t, uint8_t )', u'ns3::TracedValueCallback::Uint8')
    typehandlers.add_type_alias(u'void ( * ) ( uint8_t, uint8_t )*', u'ns3::TracedValueCallback::Uint8*')
    typehandlers.add_type_alias(u'void ( * ) ( uint8_t, uint8_t )&', u'ns3::TracedValueCallback::Uint8&')
    typehandlers.add_type_alias(u'void ( * ) ( int16_t, int16_t )', u'ns3::TracedValueCallback::Int16')
    typehandlers.add_type_alias(u'void ( * ) ( int16_t, int16_t )*', u'ns3::TracedValueCallback::Int16*')
    typehandlers.add_type_alias(u'void ( * ) ( int16_t, int16_t )&', u'ns3::TracedValueCallback::Int16&')
    typehandlers.add_type_alias(u'void ( * ) ( uint16_t, uint16_t )', u'ns3::TracedValueCallback::Uint16')
    typehandlers.add_type_alias(u'void ( * ) ( uint16_t, uint16_t )*', u'ns3::TracedValueCallback::Uint16*')
    typehandlers.add_type_alias(u'void ( * ) ( uint16_t, uint16_t )&', u'ns3::TracedValueCallback::Uint16&')
    typehandlers.add_type_alias(u'void ( * ) ( int32_t, int32_t )', u'ns3::TracedValueCallback::Int32')
    typehandlers.add_type_alias(u'void ( * ) ( int32_t, int32_t )*', u'ns3::TracedValueCallback::Int32*')
    typehandlers.add_type_alias(u'void ( * ) ( int32_t, int32_t )&', u'ns3::TracedValueCallback::Int32&')
    typehandlers.add_type_alias(u'void ( * ) ( uint32_t, uint32_t )', u'ns3::TracedValueCallback::Uint32')
    typehandlers.add_type_alias(u'void ( * ) ( uint32_t, uint32_t )*', u'ns3::TracedValueCallback::Uint32*')
    typehandlers.add_type_alias(u'void ( * ) ( uint32_t, uint32_t )&', u'ns3::TracedValueCallback::Uint32&')
    typehandlers.add_type_alias(u'void ( * ) ( double, double )', u'ns3::TracedValueCallback::Double')
    typehandlers.add_type_alias(u'void ( * ) ( double, double )*', u'ns3::TracedValueCallback::Double*')
    typehandlers.add_type_alias(u'void ( * ) ( double, double )&', u'ns3::TracedValueCallback::Double&')
    typehandlers.add_type_alias(u'void ( * ) (  )', u'ns3::TracedValueCallback::Void')
    typehandlers.add_type_alias(u'void ( * ) (  )*', u'ns3::TracedValueCallback::Void*')
    typehandlers.add_type_alias(u'void ( * ) (  )&', u'ns3::TracedValueCallback::Void&')

def register_types_ns3_internal(module):
    root_module = module.get_root()
    

def register_methods(root_module):
    register_Ns3Address_methods(root_module, root_module['ns3::Address'])
    register_Ns3ApplicationContainer_methods(root_module, root_module['ns3::ApplicationContainer'])
    register_Ns3AsciiTraceHelper_methods(root_module, root_module['ns3::AsciiTraceHelper'])
    register_Ns3AsciiTraceHelperForDevice_methods(root_module, root_module['ns3::AsciiTraceHelperForDevice'])
    register_Ns3AttributeConstructionList_methods(root_module, root_module['ns3::AttributeConstructionList'])
    register_Ns3AttributeConstructionListItem_methods(root_module, root_module['ns3::AttributeConstructionList::Item'])
    register_Ns3Buffer_methods(root_module, root_module['ns3::Buffer'])
    register_Ns3BufferIterator_methods(root_module, root_module['ns3::Buffer::Iterator'])
    register_Ns3ByteTagIterator_methods(root_module, root_module['ns3::ByteTagIterator'])
    register_Ns3ByteTagIteratorItem_methods(root_module, root_module['ns3::ByteTagIterator::Item'])
    register_Ns3ByteTagList_methods(root_module, root_module['ns3::ByteTagList'])
    register_Ns3ByteTagListIterator_methods(root_module, root_module['ns3::ByteTagList::Iterator'])
    register_Ns3ByteTagListIteratorItem_methods(root_module, root_module['ns3::ByteTagList::Iterator::Item'])
    register_Ns3CallbackBase_methods(root_module, root_module['ns3::CallbackBase'])
    register_Ns3DataRate_methods(root_module, root_module['ns3::DataRate'])
    register_Ns3DefaultDeleter__Ns3AttributeAccessor_methods(root_module, root_module['ns3::DefaultDeleter< ns3::AttributeAccessor >'])
    register_Ns3DefaultDeleter__Ns3AttributeChecker_methods(root_module, root_module['ns3::DefaultDeleter< ns3::AttributeChecker >'])
    register_Ns3DefaultDeleter__Ns3AttributeValue_methods(root_module, root_module['ns3::DefaultDeleter< ns3::AttributeValue >'])
    register_Ns3DefaultDeleter__Ns3CallbackImplBase_methods(root_module, root_module['ns3::DefaultDeleter< ns3::CallbackImplBase >'])
    register_Ns3DefaultDeleter__Ns3EventImpl_methods(root_module, root_module['ns3::DefaultDeleter< ns3::EventImpl >'])
    register_Ns3DefaultDeleter__Ns3HashImplementation_methods(root_module, root_module['ns3::DefaultDeleter< ns3::Hash::Implementation >'])
    register_Ns3DefaultDeleter__Ns3LoraInterferenceHelperEvent_methods(root_module, root_module['ns3::DefaultDeleter< ns3::LoraInterferenceHelper::Event >'])
    register_Ns3DefaultDeleter__Ns3NixVector_methods(root_module, root_module['ns3::DefaultDeleter< ns3::NixVector >'])
    register_Ns3DefaultDeleter__Ns3OutputStreamWrapper_methods(root_module, root_module['ns3::DefaultDeleter< ns3::OutputStreamWrapper >'])
    register_Ns3DefaultDeleter__Ns3Packet_methods(root_module, root_module['ns3::DefaultDeleter< ns3::Packet >'])
    register_Ns3DefaultDeleter__Ns3TraceSourceAccessor_methods(root_module, root_module['ns3::DefaultDeleter< ns3::TraceSourceAccessor >'])
    register_Ns3DeviceStatus_methods(root_module, root_module['ns3::DeviceStatus'])
    register_Ns3DeviceStatusReply_methods(root_module, root_module['ns3::DeviceStatus::Reply'])
    register_Ns3EventId_methods(root_module, root_module['ns3::EventId'])
    register_Ns3ForwarderHelper_methods(root_module, root_module['ns3::ForwarderHelper'])
    register_Ns3GatewayStatus_methods(root_module, root_module['ns3::GatewayStatus'])
    register_Ns3Hasher_methods(root_module, root_module['ns3::Hasher'])
    register_Ns3Ipv4Address_methods(root_module, root_module['ns3::Ipv4Address'])
    register_Ns3Ipv4Mask_methods(root_module, root_module['ns3::Ipv4Mask'])
    register_Ns3Ipv6Address_methods(root_module, root_module['ns3::Ipv6Address'])
    register_Ns3Ipv6Prefix_methods(root_module, root_module['ns3::Ipv6Prefix'])
    register_Ns3LoraChannelParameters_methods(root_module, root_module['ns3::LoraChannelParameters'])
    register_Ns3LoraDeviceAddress_methods(root_module, root_module['ns3::LoraDeviceAddress'])
    register_Ns3LoraHelper_methods(root_module, root_module['ns3::LoraHelper'])
    register_Ns3LoraInterferenceHelper_methods(root_module, root_module['ns3::LoraInterferenceHelper'])
    register_Ns3LoraMacHelper_methods(root_module, root_module['ns3::LoraMacHelper'])
    register_Ns3LoraPhyHelper_methods(root_module, root_module['ns3::LoraPhyHelper'])
    register_Ns3LoraTxParameters_methods(root_module, root_module['ns3::LoraTxParameters'])
    register_Ns3Mac48Address_methods(root_module, root_module['ns3::Mac48Address'])
    register_Ns3NetDeviceContainer_methods(root_module, root_module['ns3::NetDeviceContainer'])
    register_Ns3NetworkServerHelper_methods(root_module, root_module['ns3::NetworkServerHelper'])
    register_Ns3NodeContainer_methods(root_module, root_module['ns3::NodeContainer'])
    register_Ns3NwkAddr_methods(root_module, root_module['ns3::NwkAddr'])
    register_Ns3NwkID_methods(root_module, root_module['ns3::NwkID'])
    register_Ns3ObjectBase_methods(root_module, root_module['ns3::ObjectBase'])
    register_Ns3ObjectDeleter_methods(root_module, root_module['ns3::ObjectDeleter'])
    register_Ns3ObjectFactory_methods(root_module, root_module['ns3::ObjectFactory'])
    register_Ns3OneShotSenderHelper_methods(root_module, root_module['ns3::OneShotSenderHelper'])
    register_Ns3PacketMetadata_methods(root_module, root_module['ns3::PacketMetadata'])
    register_Ns3PacketMetadataItem_methods(root_module, root_module['ns3::PacketMetadata::Item'])
    register_Ns3PacketMetadataItemIterator_methods(root_module, root_module['ns3::PacketMetadata::ItemIterator'])
    register_Ns3PacketTagIterator_methods(root_module, root_module['ns3::PacketTagIterator'])
    register_Ns3PacketTagIteratorItem_methods(root_module, root_module['ns3::PacketTagIterator::Item'])
    register_Ns3PacketTagList_methods(root_module, root_module['ns3::PacketTagList'])
    register_Ns3PacketTagListTagData_methods(root_module, root_module['ns3::PacketTagList::TagData'])
    register_Ns3PcapFile_methods(root_module, root_module['ns3::PcapFile'])
    register_Ns3PcapHelper_methods(root_module, root_module['ns3::PcapHelper'])
    register_Ns3PcapHelperForDevice_methods(root_module, root_module['ns3::PcapHelperForDevice'])
    register_Ns3PeriodicSenderHelper_methods(root_module, root_module['ns3::PeriodicSenderHelper'])
    register_Ns3PointToPointHelper_methods(root_module, root_module['ns3::PointToPointHelper'])
    register_Ns3SimpleRefCount__Ns3Object_Ns3ObjectBase_Ns3ObjectDeleter_methods(root_module, root_module['ns3::SimpleRefCount< ns3::Object, ns3::ObjectBase, ns3::ObjectDeleter >'])
    register_Ns3Simulator_methods(root_module, root_module['ns3::Simulator'])
    register_Ns3Tag_methods(root_module, root_module['ns3::Tag'])
    register_Ns3TagBuffer_methods(root_module, root_module['ns3::TagBuffer'])
    register_Ns3TimeWithUnit_methods(root_module, root_module['ns3::TimeWithUnit'])
    register_Ns3TracedValue__Double_methods(root_module, root_module['ns3::TracedValue< double >'])
    register_Ns3TracedValue__Int_methods(root_module, root_module['ns3::TracedValue< int >'])
    register_Ns3TracedValue__Ns3EndDeviceLoraPhyState_methods(root_module, root_module['ns3::TracedValue< ns3::EndDeviceLoraPhy::State >'])
    register_Ns3TracedValue__Unsigned_char_methods(root_module, root_module['ns3::TracedValue< unsigned char >'])
    register_Ns3TypeId_methods(root_module, root_module['ns3::TypeId'])
    register_Ns3TypeIdAttributeInformation_methods(root_module, root_module['ns3::TypeId::AttributeInformation'])
    register_Ns3TypeIdTraceSourceInformation_methods(root_module, root_module['ns3::TypeId::TraceSourceInformation'])
    register_Ns3Vector2D_methods(root_module, root_module['ns3::Vector2D'])
    register_Ns3Vector3D_methods(root_module, root_module['ns3::Vector3D'])
    register_Ns3Empty_methods(root_module, root_module['ns3::empty'])
    register_Ns3Int64x64_t_methods(root_module, root_module['ns3::int64x64_t'])
    register_Ns3Chunk_methods(root_module, root_module['ns3::Chunk'])
    register_Ns3Header_methods(root_module, root_module['ns3::Header'])
    register_Ns3LoraFrameHeader_methods(root_module, root_module['ns3::LoraFrameHeader'])
    register_Ns3LoraMacHeader_methods(root_module, root_module['ns3::LoraMacHeader'])
    register_Ns3LoraTag_methods(root_module, root_module['ns3::LoraTag'])
    register_Ns3Object_methods(root_module, root_module['ns3::Object'])
    register_Ns3ObjectAggregateIterator_methods(root_module, root_module['ns3::Object::AggregateIterator'])
    register_Ns3PcapFileWrapper_methods(root_module, root_module['ns3::PcapFileWrapper'])
    register_Ns3PropagationDelayModel_methods(root_module, root_module['ns3::PropagationDelayModel'])
    register_Ns3PropagationLossModel_methods(root_module, root_module['ns3::PropagationLossModel'])
    register_Ns3RandomPropagationDelayModel_methods(root_module, root_module['ns3::RandomPropagationDelayModel'])
    register_Ns3RandomPropagationLossModel_methods(root_module, root_module['ns3::RandomPropagationLossModel'])
    register_Ns3RandomVariableStream_methods(root_module, root_module['ns3::RandomVariableStream'])
    register_Ns3RangePropagationLossModel_methods(root_module, root_module['ns3::RangePropagationLossModel'])
    register_Ns3SequentialRandomVariable_methods(root_module, root_module['ns3::SequentialRandomVariable'])
    register_Ns3SimpleRefCount__Ns3AttributeAccessor_Ns3Empty_Ns3DefaultDeleter__lt__ns3AttributeAccessor__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::AttributeAccessor, ns3::empty, ns3::DefaultDeleter<ns3::AttributeAccessor> >'])
    register_Ns3SimpleRefCount__Ns3AttributeChecker_Ns3Empty_Ns3DefaultDeleter__lt__ns3AttributeChecker__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::AttributeChecker, ns3::empty, ns3::DefaultDeleter<ns3::AttributeChecker> >'])
    register_Ns3SimpleRefCount__Ns3AttributeValue_Ns3Empty_Ns3DefaultDeleter__lt__ns3AttributeValue__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::AttributeValue, ns3::empty, ns3::DefaultDeleter<ns3::AttributeValue> >'])
    register_Ns3SimpleRefCount__Ns3CallbackImplBase_Ns3Empty_Ns3DefaultDeleter__lt__ns3CallbackImplBase__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::CallbackImplBase, ns3::empty, ns3::DefaultDeleter<ns3::CallbackImplBase> >'])
    register_Ns3SimpleRefCount__Ns3EventImpl_Ns3Empty_Ns3DefaultDeleter__lt__ns3EventImpl__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::EventImpl, ns3::empty, ns3::DefaultDeleter<ns3::EventImpl> >'])
    register_Ns3SimpleRefCount__Ns3HashImplementation_Ns3Empty_Ns3DefaultDeleter__lt__ns3HashImplementation__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::Hash::Implementation, ns3::empty, ns3::DefaultDeleter<ns3::Hash::Implementation> >'])
    register_Ns3SimpleRefCount__Ns3LoraInterferenceHelperEvent_Ns3Empty_Ns3DefaultDeleter__lt__ns3LoraInterferenceHelperEvent__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::LoraInterferenceHelper::Event, ns3::empty, ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event> >'])
    register_Ns3SimpleRefCount__Ns3NixVector_Ns3Empty_Ns3DefaultDeleter__lt__ns3NixVector__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::NixVector, ns3::empty, ns3::DefaultDeleter<ns3::NixVector> >'])
    register_Ns3SimpleRefCount__Ns3OutputStreamWrapper_Ns3Empty_Ns3DefaultDeleter__lt__ns3OutputStreamWrapper__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::OutputStreamWrapper, ns3::empty, ns3::DefaultDeleter<ns3::OutputStreamWrapper> >'])
    register_Ns3SimpleRefCount__Ns3Packet_Ns3Empty_Ns3DefaultDeleter__lt__ns3Packet__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::Packet, ns3::empty, ns3::DefaultDeleter<ns3::Packet> >'])
    register_Ns3SimpleRefCount__Ns3TraceSourceAccessor_Ns3Empty_Ns3DefaultDeleter__lt__ns3TraceSourceAccessor__gt___methods(root_module, root_module['ns3::SimpleRefCount< ns3::TraceSourceAccessor, ns3::empty, ns3::DefaultDeleter<ns3::TraceSourceAccessor> >'])
    register_Ns3SubBand_methods(root_module, root_module['ns3::SubBand'])
    register_Ns3ThreeLogDistancePropagationLossModel_methods(root_module, root_module['ns3::ThreeLogDistancePropagationLossModel'])
    register_Ns3Time_methods(root_module, root_module['ns3::Time'])
    register_Ns3TraceSourceAccessor_methods(root_module, root_module['ns3::TraceSourceAccessor'])
    register_Ns3Trailer_methods(root_module, root_module['ns3::Trailer'])
    register_Ns3TriangularRandomVariable_methods(root_module, root_module['ns3::TriangularRandomVariable'])
    register_Ns3TwoRayGroundPropagationLossModel_methods(root_module, root_module['ns3::TwoRayGroundPropagationLossModel'])
    register_Ns3UniformRandomVariable_methods(root_module, root_module['ns3::UniformRandomVariable'])
    register_Ns3WeibullRandomVariable_methods(root_module, root_module['ns3::WeibullRandomVariable'])
    register_Ns3ZetaRandomVariable_methods(root_module, root_module['ns3::ZetaRandomVariable'])
    register_Ns3ZipfRandomVariable_methods(root_module, root_module['ns3::ZipfRandomVariable'])
    register_Ns3Application_methods(root_module, root_module['ns3::Application'])
    register_Ns3AttributeAccessor_methods(root_module, root_module['ns3::AttributeAccessor'])
    register_Ns3AttributeChecker_methods(root_module, root_module['ns3::AttributeChecker'])
    register_Ns3AttributeValue_methods(root_module, root_module['ns3::AttributeValue'])
    register_Ns3BooleanChecker_methods(root_module, root_module['ns3::BooleanChecker'])
    register_Ns3BooleanValue_methods(root_module, root_module['ns3::BooleanValue'])
    register_Ns3CallbackChecker_methods(root_module, root_module['ns3::CallbackChecker'])
    register_Ns3CallbackImplBase_methods(root_module, root_module['ns3::CallbackImplBase'])
    register_Ns3CallbackValue_methods(root_module, root_module['ns3::CallbackValue'])
    register_Ns3Channel_methods(root_module, root_module['ns3::Channel'])
    register_Ns3ConstantRandomVariable_methods(root_module, root_module['ns3::ConstantRandomVariable'])
    register_Ns3ConstantSpeedPropagationDelayModel_methods(root_module, root_module['ns3::ConstantSpeedPropagationDelayModel'])
    register_Ns3DataRateChecker_methods(root_module, root_module['ns3::DataRateChecker'])
    register_Ns3DataRateValue_methods(root_module, root_module['ns3::DataRateValue'])
    register_Ns3DeterministicRandomVariable_methods(root_module, root_module['ns3::DeterministicRandomVariable'])
    register_Ns3DoubleValue_methods(root_module, root_module['ns3::DoubleValue'])
    register_Ns3EmpiricalRandomVariable_methods(root_module, root_module['ns3::EmpiricalRandomVariable'])
    register_Ns3EmptyAttributeAccessor_methods(root_module, root_module['ns3::EmptyAttributeAccessor'])
    register_Ns3EmptyAttributeChecker_methods(root_module, root_module['ns3::EmptyAttributeChecker'])
    register_Ns3EmptyAttributeValue_methods(root_module, root_module['ns3::EmptyAttributeValue'])
    register_Ns3EnumChecker_methods(root_module, root_module['ns3::EnumChecker'])
    register_Ns3EnumValue_methods(root_module, root_module['ns3::EnumValue'])
    register_Ns3ErlangRandomVariable_methods(root_module, root_module['ns3::ErlangRandomVariable'])
    register_Ns3EventImpl_methods(root_module, root_module['ns3::EventImpl'])
    register_Ns3ExponentialRandomVariable_methods(root_module, root_module['ns3::ExponentialRandomVariable'])
    register_Ns3FixedRssLossModel_methods(root_module, root_module['ns3::FixedRssLossModel'])
    register_Ns3Forwarder_methods(root_module, root_module['ns3::Forwarder'])
    register_Ns3FriisPropagationLossModel_methods(root_module, root_module['ns3::FriisPropagationLossModel'])
    register_Ns3GammaRandomVariable_methods(root_module, root_module['ns3::GammaRandomVariable'])
    register_Ns3IntegerValue_methods(root_module, root_module['ns3::IntegerValue'])
    register_Ns3Ipv4AddressChecker_methods(root_module, root_module['ns3::Ipv4AddressChecker'])
    register_Ns3Ipv4AddressValue_methods(root_module, root_module['ns3::Ipv4AddressValue'])
    register_Ns3Ipv4MaskChecker_methods(root_module, root_module['ns3::Ipv4MaskChecker'])
    register_Ns3Ipv4MaskValue_methods(root_module, root_module['ns3::Ipv4MaskValue'])
    register_Ns3Ipv6AddressChecker_methods(root_module, root_module['ns3::Ipv6AddressChecker'])
    register_Ns3Ipv6AddressValue_methods(root_module, root_module['ns3::Ipv6AddressValue'])
    register_Ns3Ipv6PrefixChecker_methods(root_module, root_module['ns3::Ipv6PrefixChecker'])
    register_Ns3Ipv6PrefixValue_methods(root_module, root_module['ns3::Ipv6PrefixValue'])
    register_Ns3LogDistancePropagationLossModel_methods(root_module, root_module['ns3::LogDistancePropagationLossModel'])
    register_Ns3LogNormalRandomVariable_methods(root_module, root_module['ns3::LogNormalRandomVariable'])
    register_Ns3LogicalLoraChannel_methods(root_module, root_module['ns3::LogicalLoraChannel'])
    register_Ns3LogicalLoraChannelHelper_methods(root_module, root_module['ns3::LogicalLoraChannelHelper'])
    register_Ns3LoraChannel_methods(root_module, root_module['ns3::LoraChannel'])
    register_Ns3LoraDeviceAddressGenerator_methods(root_module, root_module['ns3::LoraDeviceAddressGenerator'])
    register_Ns3LoraMac_methods(root_module, root_module['ns3::LoraMac'])
    register_Ns3LoraPhy_methods(root_module, root_module['ns3::LoraPhy'])
    register_Ns3Mac48AddressChecker_methods(root_module, root_module['ns3::Mac48AddressChecker'])
    register_Ns3Mac48AddressValue_methods(root_module, root_module['ns3::Mac48AddressValue'])
    register_Ns3MacCommand_methods(root_module, root_module['ns3::MacCommand'])
    register_Ns3MatrixPropagationLossModel_methods(root_module, root_module['ns3::MatrixPropagationLossModel'])
    register_Ns3MobilityModel_methods(root_module, root_module['ns3::MobilityModel'])
    register_Ns3NakagamiPropagationLossModel_methods(root_module, root_module['ns3::NakagamiPropagationLossModel'])
    register_Ns3NetDevice_methods(root_module, root_module['ns3::NetDevice'])
    register_Ns3NewChannelAns_methods(root_module, root_module['ns3::NewChannelAns'])
    register_Ns3NewChannelReq_methods(root_module, root_module['ns3::NewChannelReq'])
    register_Ns3NixVector_methods(root_module, root_module['ns3::NixVector'])
    register_Ns3Node_methods(root_module, root_module['ns3::Node'])
    register_Ns3NormalRandomVariable_methods(root_module, root_module['ns3::NormalRandomVariable'])
    register_Ns3ObjectFactoryChecker_methods(root_module, root_module['ns3::ObjectFactoryChecker'])
    register_Ns3ObjectFactoryValue_methods(root_module, root_module['ns3::ObjectFactoryValue'])
    register_Ns3OneShotSender_methods(root_module, root_module['ns3::OneShotSender'])
    register_Ns3OutputStreamWrapper_methods(root_module, root_module['ns3::OutputStreamWrapper'])
    register_Ns3Packet_methods(root_module, root_module['ns3::Packet'])
    register_Ns3ParetoRandomVariable_methods(root_module, root_module['ns3::ParetoRandomVariable'])
    register_Ns3PeriodicSender_methods(root_module, root_module['ns3::PeriodicSender'])
    register_Ns3PointToPointNetDevice_methods(root_module, root_module['ns3::PointToPointNetDevice'])
    register_Ns3RxParamSetupAns_methods(root_module, root_module['ns3::RxParamSetupAns'])
    register_Ns3RxParamSetupReq_methods(root_module, root_module['ns3::RxParamSetupReq'])
    register_Ns3RxTimingSetupAns_methods(root_module, root_module['ns3::RxTimingSetupAns'])
    register_Ns3RxTimingSetupReq_methods(root_module, root_module['ns3::RxTimingSetupReq'])
    register_Ns3SimpleNetworkServer_methods(root_module, root_module['ns3::SimpleNetworkServer'])
    register_Ns3TimeValue_methods(root_module, root_module['ns3::TimeValue'])
    register_Ns3TxParamSetupAns_methods(root_module, root_module['ns3::TxParamSetupAns'])
    register_Ns3TxParamSetupReq_methods(root_module, root_module['ns3::TxParamSetupReq'])
    register_Ns3TypeIdChecker_methods(root_module, root_module['ns3::TypeIdChecker'])
    register_Ns3TypeIdValue_methods(root_module, root_module['ns3::TypeIdValue'])
    register_Ns3UintegerValue_methods(root_module, root_module['ns3::UintegerValue'])
    register_Ns3Vector2DChecker_methods(root_module, root_module['ns3::Vector2DChecker'])
    register_Ns3Vector2DValue_methods(root_module, root_module['ns3::Vector2DValue'])
    register_Ns3Vector3DChecker_methods(root_module, root_module['ns3::Vector3DChecker'])
    register_Ns3Vector3DValue_methods(root_module, root_module['ns3::Vector3DValue'])
    register_Ns3AddressChecker_methods(root_module, root_module['ns3::AddressChecker'])
    register_Ns3AddressValue_methods(root_module, root_module['ns3::AddressValue'])
    register_Ns3CallbackImpl__Bool_Ns3Ptr__lt__ns3NetDevice__gt___Ns3Ptr__lt__const_ns3Packet__gt___Unsigned_short_Const_ns3Address___amp___Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< bool, ns3::Ptr<ns3::NetDevice>, ns3::Ptr<const ns3::Packet>, unsigned short, const ns3::Address &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Ns3ObjectBase___star___Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< ns3::ObjectBase *, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Double_Double_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, double, double, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Int_Int_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, int, int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3EndDeviceLoraPhyState_Ns3EndDeviceLoraPhyState_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::EndDeviceLoraPhy::State, ns3::EndDeviceLoraPhy::State, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3Ptr__lt__const_ns3MobilityModel__gt___Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::Ptr<const ns3::MobilityModel>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3Ptr__lt__const_ns3Packet__gt___Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::Ptr<const ns3::Packet>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3Ptr__lt__const_ns3Packet__gt___Unsigned_int_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::Ptr<const ns3::Packet>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3Ptr__lt__ns3NetDevice__gt___Ns3Ptr__lt__const_ns3Packet__gt___Unsigned_short_Const_ns3Address___amp___Const_ns3Address___amp___Ns3NetDevicePacketType_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::Ptr<ns3::NetDevice>, ns3::Ptr<const ns3::Packet>, unsigned short, const ns3::Address &, const ns3::Address &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3Ptr__lt__ns3NetDevice__gt___Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::Ptr<ns3::NetDevice>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3CallbackImpl__Void_Unsigned_char_Unsigned_char_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_Ns3Empty_methods(root_module, root_module['ns3::CallbackImpl< void, unsigned char, unsigned char, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >'])
    register_Ns3DevStatusAns_methods(root_module, root_module['ns3::DevStatusAns'])
    register_Ns3DevStatusReq_methods(root_module, root_module['ns3::DevStatusReq'])
    register_Ns3DlChannelAns_methods(root_module, root_module['ns3::DlChannelAns'])
    register_Ns3DutyCycleAns_methods(root_module, root_module['ns3::DutyCycleAns'])
    register_Ns3DutyCycleReq_methods(root_module, root_module['ns3::DutyCycleReq'])
    register_Ns3EndDeviceLoraMac_methods(root_module, root_module['ns3::EndDeviceLoraMac'])
    register_Ns3EndDeviceLoraPhy_methods(root_module, root_module['ns3::EndDeviceLoraPhy'])
    register_Ns3GatewayLoraMac_methods(root_module, root_module['ns3::GatewayLoraMac'])
    register_Ns3GatewayLoraPhy_methods(root_module, root_module['ns3::GatewayLoraPhy'])
    register_Ns3LinkAdrAns_methods(root_module, root_module['ns3::LinkAdrAns'])
    register_Ns3LinkAdrReq_methods(root_module, root_module['ns3::LinkAdrReq'])
    register_Ns3LinkCheckAns_methods(root_module, root_module['ns3::LinkCheckAns'])
    register_Ns3LinkCheckReq_methods(root_module, root_module['ns3::LinkCheckReq'])
    register_Ns3LoraNetDevice_methods(root_module, root_module['ns3::LoraNetDevice'])
    register_Ns3HashImplementation_methods(root_module, root_module['ns3::Hash::Implementation'])
    register_Ns3HashFunctionFnv1a_methods(root_module, root_module['ns3::Hash::Function::Fnv1a'])
    register_Ns3HashFunctionHash32_methods(root_module, root_module['ns3::Hash::Function::Hash32'])
    register_Ns3HashFunctionHash64_methods(root_module, root_module['ns3::Hash::Function::Hash64'])
    register_Ns3HashFunctionMurmur3_methods(root_module, root_module['ns3::Hash::Function::Murmur3'])
    return

def register_Ns3Address_methods(root_module, cls):
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    cls.add_output_stream_operator()
    ## address.h (module 'network'): ns3::Address::Address() [constructor]
    cls.add_constructor([])
    ## address.h (module 'network'): ns3::Address::Address(uint8_t type, uint8_t const * buffer, uint8_t len) [constructor]
    cls.add_constructor([param('uint8_t', 'type'), param('uint8_t const *', 'buffer'), param('uint8_t', 'len')])
    ## address.h (module 'network'): ns3::Address::Address(ns3::Address const & address) [constructor]
    cls.add_constructor([param('ns3::Address const &', 'address')])
    ## address.h (module 'network'): bool ns3::Address::CheckCompatible(uint8_t type, uint8_t len) const [member function]
    cls.add_method('CheckCompatible', 
                   'bool', 
                   [param('__uint8_t', 'type'), param('__uint8_t', 'len')], 
                   is_const=True)
    ## address.h (module 'network'): uint32_t ns3::Address::CopyAllFrom(uint8_t const * buffer, uint8_t len) [member function]
    cls.add_method('CopyAllFrom', 
                   'uint32_t', 
                   [param('uint8_t const *', 'buffer'), param('__uint8_t', 'len')])
    ## address.h (module 'network'): uint32_t ns3::Address::CopyAllTo(uint8_t * buffer, uint8_t len) const [member function]
    cls.add_method('CopyAllTo', 
                   'uint32_t', 
                   [param('uint8_t *', 'buffer'), param('__uint8_t', 'len')], 
                   is_const=True)
    ## address.h (module 'network'): uint32_t ns3::Address::CopyFrom(uint8_t const * buffer, uint8_t len) [member function]
    cls.add_method('CopyFrom', 
                   'uint32_t', 
                   [param('uint8_t const *', 'buffer'), param('__uint8_t', 'len')])
    ## address.h (module 'network'): uint32_t ns3::Address::CopyTo(uint8_t * buffer) const [member function]
    cls.add_method('CopyTo', 
                   'uint32_t', 
                   [param('uint8_t *', 'buffer')], 
                   is_const=True)
    ## address.h (module 'network'): void ns3::Address::Deserialize(ns3::TagBuffer buffer) [member function]
    cls.add_method('Deserialize', 
                   'void', 
                   [param('ns3::TagBuffer', 'buffer')])
    ## address.h (module 'network'): uint8_t ns3::Address::GetLength() const [member function]
    cls.add_method('GetLength', 
                   'uint8_t', 
                   [], 
                   is_const=True)
    ## address.h (module 'network'): uint32_t ns3::Address::GetSerializedSize() const [member function]
    cls.add_method('GetSerializedSize', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## address.h (module 'network'): bool ns3::Address::IsInvalid() const [member function]
    cls.add_method('IsInvalid', 
                   'bool', 
                   [], 
                   is_const=True)
    ## address.h (module 'network'): bool ns3::Address::IsMatchingType(uint8_t type) const [member function]
    cls.add_method('IsMatchingType', 
                   'bool', 
                   [param('__uint8_t', 'type')], 
                   is_const=True)
    ## address.h (module 'network'): static uint8_t ns3::Address::Register() [member function]
    cls.add_method('Register', 
                   'uint8_t', 
                   [], 
                   is_static=True)
    ## address.h (module 'network'): void ns3::Address::Serialize(ns3::TagBuffer buffer) const [member function]
    cls.add_method('Serialize', 
                   'void', 
                   [param('ns3::TagBuffer', 'buffer')], 
                   is_const=True)
    return

def register_Ns3ApplicationContainer_methods(root_module, cls):
    ## application-container.h (module 'network'): ns3::ApplicationContainer::ApplicationContainer(ns3::ApplicationContainer const & arg0) [constructor]
    cls.add_constructor([param('ns3::ApplicationContainer const &', 'arg0')])
    ## application-container.h (module 'network'): ns3::ApplicationContainer::ApplicationContainer() [constructor]
    cls.add_constructor([])
    ## application-container.h (module 'network'): ns3::ApplicationContainer::ApplicationContainer(ns3::Ptr<ns3::Application> application) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::Application >', 'application')])
    ## application-container.h (module 'network'): ns3::ApplicationContainer::ApplicationContainer(std::string name) [constructor]
    cls.add_constructor([param('std::string', 'name')])
    ## application-container.h (module 'network'): void ns3::ApplicationContainer::Add(ns3::ApplicationContainer other) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::ApplicationContainer', 'other')])
    ## application-container.h (module 'network'): void ns3::ApplicationContainer::Add(ns3::Ptr<ns3::Application> application) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::Ptr< ns3::Application >', 'application')])
    ## application-container.h (module 'network'): void ns3::ApplicationContainer::Add(std::string name) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('std::string', 'name')])
    ## application-container.h (module 'network'): ns3::ApplicationContainer::Iterator ns3::ApplicationContainer::Begin() const [member function]
    cls.add_method('Begin', 
                   'ns3::ApplicationContainer::Iterator', 
                   [], 
                   is_const=True)
    ## application-container.h (module 'network'): ns3::ApplicationContainer::Iterator ns3::ApplicationContainer::End() const [member function]
    cls.add_method('End', 
                   'ns3::ApplicationContainer::Iterator', 
                   [], 
                   is_const=True)
    ## application-container.h (module 'network'): ns3::Ptr<ns3::Application> ns3::ApplicationContainer::Get(uint32_t i) const [member function]
    cls.add_method('Get', 
                   'ns3::Ptr< ns3::Application >', 
                   [param('__uint32_t', 'i')], 
                   is_const=True)
    ## application-container.h (module 'network'): uint32_t ns3::ApplicationContainer::GetN() const [member function]
    cls.add_method('GetN', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## application-container.h (module 'network'): void ns3::ApplicationContainer::Start(ns3::Time start) [member function]
    cls.add_method('Start', 
                   'void', 
                   [param('ns3::Time', 'start')])
    ## application-container.h (module 'network'): void ns3::ApplicationContainer::Stop(ns3::Time stop) [member function]
    cls.add_method('Stop', 
                   'void', 
                   [param('ns3::Time', 'stop')])
    return

def register_Ns3AsciiTraceHelper_methods(root_module, cls):
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelper::AsciiTraceHelper(ns3::AsciiTraceHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::AsciiTraceHelper const &', 'arg0')])
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelper::AsciiTraceHelper() [constructor]
    cls.add_constructor([])
    ## trace-helper.h (module 'network'): ns3::Ptr<ns3::OutputStreamWrapper> ns3::AsciiTraceHelper::CreateFileStream(std::string filename, std::ios_base::openmode filemode=std::ios_base::out) [member function]
    cls.add_method('CreateFileStream', 
                   'ns3::Ptr< ns3::OutputStreamWrapper >', 
                   [param('std::string', 'filename'), param('std::_Ios_Openmode', 'filemode', default_value='std::ios_base::out')])
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultDequeueSinkWithContext(ns3::Ptr<ns3::OutputStreamWrapper> file, std::string context, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultDequeueSinkWithContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('std::string', 'context'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultDequeueSinkWithoutContext(ns3::Ptr<ns3::OutputStreamWrapper> file, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultDequeueSinkWithoutContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultDropSinkWithContext(ns3::Ptr<ns3::OutputStreamWrapper> file, std::string context, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultDropSinkWithContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('std::string', 'context'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultDropSinkWithoutContext(ns3::Ptr<ns3::OutputStreamWrapper> file, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultDropSinkWithoutContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultEnqueueSinkWithContext(ns3::Ptr<ns3::OutputStreamWrapper> file, std::string context, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultEnqueueSinkWithContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('std::string', 'context'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultEnqueueSinkWithoutContext(ns3::Ptr<ns3::OutputStreamWrapper> file, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultEnqueueSinkWithoutContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultReceiveSinkWithContext(ns3::Ptr<ns3::OutputStreamWrapper> file, std::string context, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultReceiveSinkWithContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('std::string', 'context'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): static void ns3::AsciiTraceHelper::DefaultReceiveSinkWithoutContext(ns3::Ptr<ns3::OutputStreamWrapper> file, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('DefaultReceiveSinkWithoutContext', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'file'), param('ns3::Ptr< ns3::Packet const >', 'p')], 
                   is_static=True)
    ## trace-helper.h (module 'network'): std::string ns3::AsciiTraceHelper::GetFilenameFromDevice(std::string prefix, ns3::Ptr<ns3::NetDevice> device, bool useObjectNames=true) [member function]
    cls.add_method('GetFilenameFromDevice', 
                   'std::string', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'device'), param('bool', 'useObjectNames', default_value='true')])
    ## trace-helper.h (module 'network'): std::string ns3::AsciiTraceHelper::GetFilenameFromInterfacePair(std::string prefix, ns3::Ptr<ns3::Object> object, uint32_t interface, bool useObjectNames=true) [member function]
    cls.add_method('GetFilenameFromInterfacePair', 
                   'std::string', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::Object >', 'object'), param('__uint32_t', 'interface'), param('bool', 'useObjectNames', default_value='true')])
    return

def register_Ns3AsciiTraceHelperForDevice_methods(root_module, cls):
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelperForDevice::AsciiTraceHelperForDevice(ns3::AsciiTraceHelperForDevice const & arg0) [constructor]
    cls.add_constructor([param('ns3::AsciiTraceHelperForDevice const &', 'arg0')])
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelperForDevice::AsciiTraceHelperForDevice() [constructor]
    cls.add_constructor([])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(std::string prefix, ns3::Ptr<ns3::NetDevice> nd, bool explicitFilename=false) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'nd'), param('bool', 'explicitFilename', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(ns3::Ptr<ns3::OutputStreamWrapper> stream, ns3::Ptr<ns3::NetDevice> nd) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('ns3::Ptr< ns3::NetDevice >', 'nd')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(std::string prefix, std::string ndName, bool explicitFilename=false) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('std::string', 'prefix'), param('std::string', 'ndName'), param('bool', 'explicitFilename', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(ns3::Ptr<ns3::OutputStreamWrapper> stream, std::string ndName) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('std::string', 'ndName')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(std::string prefix, ns3::NetDeviceContainer d) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::NetDeviceContainer', 'd')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(ns3::Ptr<ns3::OutputStreamWrapper> stream, ns3::NetDeviceContainer d) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('ns3::NetDeviceContainer', 'd')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(std::string prefix, ns3::NodeContainer n) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::NodeContainer', 'n')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(ns3::Ptr<ns3::OutputStreamWrapper> stream, ns3::NodeContainer n) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('ns3::NodeContainer', 'n')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(std::string prefix, uint32_t nodeid, uint32_t deviceid, bool explicitFilename) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('std::string', 'prefix'), param('__uint32_t', 'nodeid'), param('__uint32_t', 'deviceid'), param('bool', 'explicitFilename')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAscii(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t nodeid, uint32_t deviceid) [member function]
    cls.add_method('EnableAscii', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('__uint32_t', 'nodeid'), param('__uint32_t', 'deviceid')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAsciiAll(std::string prefix) [member function]
    cls.add_method('EnableAsciiAll', 
                   'void', 
                   [param('std::string', 'prefix')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAsciiAll(ns3::Ptr<ns3::OutputStreamWrapper> stream) [member function]
    cls.add_method('EnableAsciiAll', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream')])
    ## trace-helper.h (module 'network'): void ns3::AsciiTraceHelperForDevice::EnableAsciiInternal(ns3::Ptr<ns3::OutputStreamWrapper> stream, std::string prefix, ns3::Ptr<ns3::NetDevice> nd, bool explicitFilename) [member function]
    cls.add_method('EnableAsciiInternal', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'nd'), param('bool', 'explicitFilename')], 
                   is_pure_virtual=True, is_virtual=True)
    return

def register_Ns3AttributeConstructionList_methods(root_module, cls):
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::AttributeConstructionList(ns3::AttributeConstructionList const & arg0) [constructor]
    cls.add_constructor([param('ns3::AttributeConstructionList const &', 'arg0')])
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::AttributeConstructionList() [constructor]
    cls.add_constructor([])
    ## attribute-construction-list.h (module 'core'): void ns3::AttributeConstructionList::Add(std::string name, ns3::Ptr<const ns3::AttributeChecker> checker, ns3::Ptr<ns3::AttributeValue> value) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::Ptr< ns3::AttributeChecker const >', 'checker'), param('ns3::Ptr< ns3::AttributeValue >', 'value')])
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::CIterator ns3::AttributeConstructionList::Begin() const [member function]
    cls.add_method('Begin', 
                   'ns3::AttributeConstructionList::CIterator', 
                   [], 
                   is_const=True)
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::CIterator ns3::AttributeConstructionList::End() const [member function]
    cls.add_method('End', 
                   'ns3::AttributeConstructionList::CIterator', 
                   [], 
                   is_const=True)
    ## attribute-construction-list.h (module 'core'): ns3::Ptr<ns3::AttributeValue> ns3::AttributeConstructionList::Find(ns3::Ptr<const ns3::AttributeChecker> checker) const [member function]
    cls.add_method('Find', 
                   'ns3::Ptr< ns3::AttributeValue >', 
                   [param('ns3::Ptr< ns3::AttributeChecker const >', 'checker')], 
                   is_const=True)
    return

def register_Ns3AttributeConstructionListItem_methods(root_module, cls):
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item::Item() [constructor]
    cls.add_constructor([])
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item::Item(ns3::AttributeConstructionList::Item const & arg0) [constructor]
    cls.add_constructor([param('ns3::AttributeConstructionList::Item const &', 'arg0')])
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item::checker [variable]
    cls.add_instance_attribute('checker', 'ns3::Ptr< ns3::AttributeChecker const >', is_const=False)
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item::name [variable]
    cls.add_instance_attribute('name', 'std::string', is_const=False)
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item::value [variable]
    cls.add_instance_attribute('value', 'ns3::Ptr< ns3::AttributeValue >', is_const=False)
    return

def register_Ns3Buffer_methods(root_module, cls):
    ## buffer.h (module 'network'): ns3::Buffer::Buffer(ns3::Buffer const & o) [constructor]
    cls.add_constructor([param('ns3::Buffer const &', 'o')])
    ## buffer.h (module 'network'): ns3::Buffer::Buffer() [constructor]
    cls.add_constructor([])
    ## buffer.h (module 'network'): ns3::Buffer::Buffer(uint32_t dataSize) [constructor]
    cls.add_constructor([param('uint32_t', 'dataSize')])
    ## buffer.h (module 'network'): ns3::Buffer::Buffer(uint32_t dataSize, bool initialize) [constructor]
    cls.add_constructor([param('uint32_t', 'dataSize'), param('bool', 'initialize')])
    ## buffer.h (module 'network'): void ns3::Buffer::AddAtEnd(uint32_t end) [member function]
    cls.add_method('AddAtEnd', 
                   'void', 
                   [param('__uint32_t', 'end')])
    ## buffer.h (module 'network'): void ns3::Buffer::AddAtEnd(ns3::Buffer const & o) [member function]
    cls.add_method('AddAtEnd', 
                   'void', 
                   [param('ns3::Buffer const &', 'o')])
    ## buffer.h (module 'network'): void ns3::Buffer::AddAtStart(uint32_t start) [member function]
    cls.add_method('AddAtStart', 
                   'void', 
                   [param('__uint32_t', 'start')])
    ## buffer.h (module 'network'): ns3::Buffer::Iterator ns3::Buffer::Begin() const [member function]
    cls.add_method('Begin', 
                   'ns3::Buffer::Iterator', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): void ns3::Buffer::CopyData(std::ostream * os, uint32_t size) const [member function]
    cls.add_method('CopyData', 
                   'void', 
                   [param('std::ostream *', 'os'), param('__uint32_t', 'size')], 
                   is_const=True)
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::CopyData(uint8_t * buffer, uint32_t size) const [member function]
    cls.add_method('CopyData', 
                   'uint32_t', 
                   [param('uint8_t *', 'buffer'), param('__uint32_t', 'size')], 
                   is_const=True)
    ## buffer.h (module 'network'): ns3::Buffer ns3::Buffer::CreateFragment(uint32_t start, uint32_t length) const [member function]
    cls.add_method('CreateFragment', 
                   'ns3::Buffer', 
                   [param('__uint32_t', 'start'), param('__uint32_t', 'length')], 
                   is_const=True)
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Deserialize(uint8_t const * buffer, uint32_t size) [member function]
    cls.add_method('Deserialize', 
                   'uint32_t', 
                   [param('uint8_t const *', 'buffer'), param('__uint32_t', 'size')])
    ## buffer.h (module 'network'): ns3::Buffer::Iterator ns3::Buffer::End() const [member function]
    cls.add_method('End', 
                   'ns3::Buffer::Iterator', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::GetSerializedSize() const [member function]
    cls.add_method('GetSerializedSize', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::GetSize() const [member function]
    cls.add_method('GetSize', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): uint8_t const * ns3::Buffer::PeekData() const [member function]
    cls.add_method('PeekData', 
                   'uint8_t const *', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): void ns3::Buffer::RemoveAtEnd(uint32_t end) [member function]
    cls.add_method('RemoveAtEnd', 
                   'void', 
                   [param('__uint32_t', 'end')])
    ## buffer.h (module 'network'): void ns3::Buffer::RemoveAtStart(uint32_t start) [member function]
    cls.add_method('RemoveAtStart', 
                   'void', 
                   [param('__uint32_t', 'start')])
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Serialize(uint8_t * buffer, uint32_t maxSize) const [member function]
    cls.add_method('Serialize', 
                   'uint32_t', 
                   [param('uint8_t *', 'buffer'), param('__uint32_t', 'maxSize')], 
                   is_const=True)
    return

def register_Ns3BufferIterator_methods(root_module, cls):
    ## buffer.h (module 'network'): ns3::Buffer::Iterator::Iterator(ns3::Buffer::Iterator const & arg0) [constructor]
    cls.add_constructor([param('ns3::Buffer::Iterator const &', 'arg0')])
    ## buffer.h (module 'network'): ns3::Buffer::Iterator::Iterator() [constructor]
    cls.add_constructor([])
    ## buffer.h (module 'network'): uint16_t ns3::Buffer::Iterator::CalculateIpChecksum(uint16_t size) [member function]
    cls.add_method('CalculateIpChecksum', 
                   'uint16_t', 
                   [param('__uint16_t', 'size')])
    ## buffer.h (module 'network'): uint16_t ns3::Buffer::Iterator::CalculateIpChecksum(uint16_t size, uint32_t initialChecksum) [member function]
    cls.add_method('CalculateIpChecksum', 
                   'uint16_t', 
                   [param('__uint16_t', 'size'), param('__uint32_t', 'initialChecksum')])
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Iterator::GetDistanceFrom(ns3::Buffer::Iterator const & o) const [member function]
    cls.add_method('GetDistanceFrom', 
                   'uint32_t', 
                   [param('ns3::Buffer::Iterator const &', 'o')], 
                   is_const=True)
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Iterator::GetRemainingSize() const [member function]
    cls.add_method('GetRemainingSize', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Iterator::GetSize() const [member function]
    cls.add_method('GetSize', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): bool ns3::Buffer::Iterator::IsEnd() const [member function]
    cls.add_method('IsEnd', 
                   'bool', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): bool ns3::Buffer::Iterator::IsStart() const [member function]
    cls.add_method('IsStart', 
                   'bool', 
                   [], 
                   is_const=True)
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Next() [member function]
    cls.add_method('Next', 
                   'void', 
                   [])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Next(uint32_t delta) [member function]
    cls.add_method('Next', 
                   'void', 
                   [param('__uint32_t', 'delta')])
    ## buffer.h (module 'network'): uint8_t ns3::Buffer::Iterator::PeekU8() [member function]
    cls.add_method('PeekU8', 
                   'uint8_t', 
                   [])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Prev() [member function]
    cls.add_method('Prev', 
                   'void', 
                   [])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Prev(uint32_t delta) [member function]
    cls.add_method('Prev', 
                   'void', 
                   [param('__uint32_t', 'delta')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Read(uint8_t * buffer, uint32_t size) [member function]
    cls.add_method('Read', 
                   'void', 
                   [param('uint8_t *', 'buffer'), param('__uint32_t', 'size')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Read(ns3::Buffer::Iterator start, uint32_t size) [member function]
    cls.add_method('Read', 
                   'void', 
                   [param('ns3::Buffer::Iterator', 'start'), param('__uint32_t', 'size')])
    ## buffer.h (module 'network'): uint16_t ns3::Buffer::Iterator::ReadLsbtohU16() [member function]
    cls.add_method('ReadLsbtohU16', 
                   'uint16_t', 
                   [])
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Iterator::ReadLsbtohU32() [member function]
    cls.add_method('ReadLsbtohU32', 
                   'uint32_t', 
                   [])
    ## buffer.h (module 'network'): uint64_t ns3::Buffer::Iterator::ReadLsbtohU64() [member function]
    cls.add_method('ReadLsbtohU64', 
                   'uint64_t', 
                   [])
    ## buffer.h (module 'network'): uint16_t ns3::Buffer::Iterator::ReadNtohU16() [member function]
    cls.add_method('ReadNtohU16', 
                   'uint16_t', 
                   [])
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Iterator::ReadNtohU32() [member function]
    cls.add_method('ReadNtohU32', 
                   'uint32_t', 
                   [])
    ## buffer.h (module 'network'): uint64_t ns3::Buffer::Iterator::ReadNtohU64() [member function]
    cls.add_method('ReadNtohU64', 
                   'uint64_t', 
                   [])
    ## buffer.h (module 'network'): uint16_t ns3::Buffer::Iterator::ReadU16() [member function]
    cls.add_method('ReadU16', 
                   'uint16_t', 
                   [])
    ## buffer.h (module 'network'): uint32_t ns3::Buffer::Iterator::ReadU32() [member function]
    cls.add_method('ReadU32', 
                   'uint32_t', 
                   [])
    ## buffer.h (module 'network'): uint64_t ns3::Buffer::Iterator::ReadU64() [member function]
    cls.add_method('ReadU64', 
                   'uint64_t', 
                   [])
    ## buffer.h (module 'network'): uint8_t ns3::Buffer::Iterator::ReadU8() [member function]
    cls.add_method('ReadU8', 
                   'uint8_t', 
                   [])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Write(uint8_t const * buffer, uint32_t size) [member function]
    cls.add_method('Write', 
                   'void', 
                   [param('uint8_t const *', 'buffer'), param('__uint32_t', 'size')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::Write(ns3::Buffer::Iterator start, ns3::Buffer::Iterator end) [member function]
    cls.add_method('Write', 
                   'void', 
                   [param('ns3::Buffer::Iterator', 'start'), param('ns3::Buffer::Iterator', 'end')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteHtolsbU16(uint16_t data) [member function]
    cls.add_method('WriteHtolsbU16', 
                   'void', 
                   [param('__uint16_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteHtolsbU32(uint32_t data) [member function]
    cls.add_method('WriteHtolsbU32', 
                   'void', 
                   [param('__uint32_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteHtolsbU64(uint64_t data) [member function]
    cls.add_method('WriteHtolsbU64', 
                   'void', 
                   [param('__uint64_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteHtonU16(uint16_t data) [member function]
    cls.add_method('WriteHtonU16', 
                   'void', 
                   [param('__uint16_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteHtonU32(uint32_t data) [member function]
    cls.add_method('WriteHtonU32', 
                   'void', 
                   [param('__uint32_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteHtonU64(uint64_t data) [member function]
    cls.add_method('WriteHtonU64', 
                   'void', 
                   [param('__uint64_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteU16(uint16_t data) [member function]
    cls.add_method('WriteU16', 
                   'void', 
                   [param('__uint16_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteU32(uint32_t data) [member function]
    cls.add_method('WriteU32', 
                   'void', 
                   [param('__uint32_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteU64(uint64_t data) [member function]
    cls.add_method('WriteU64', 
                   'void', 
                   [param('__uint64_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteU8(uint8_t data) [member function]
    cls.add_method('WriteU8', 
                   'void', 
                   [param('__uint8_t', 'data')])
    ## buffer.h (module 'network'): void ns3::Buffer::Iterator::WriteU8(uint8_t data, uint32_t len) [member function]
    cls.add_method('WriteU8', 
                   'void', 
                   [param('__uint8_t', 'data'), param('__uint32_t', 'len')])
    return

def register_Ns3ByteTagIterator_methods(root_module, cls):
    ## packet.h (module 'network'): ns3::ByteTagIterator::ByteTagIterator(ns3::ByteTagIterator const & arg0) [constructor]
    cls.add_constructor([param('ns3::ByteTagIterator const &', 'arg0')])
    ## packet.h (module 'network'): bool ns3::ByteTagIterator::HasNext() const [member function]
    cls.add_method('HasNext', 
                   'bool', 
                   [], 
                   is_const=True)
    ## packet.h (module 'network'): ns3::ByteTagIterator::Item ns3::ByteTagIterator::Next() [member function]
    cls.add_method('Next', 
                   'ns3::ByteTagIterator::Item', 
                   [])
    return

def register_Ns3ByteTagIteratorItem_methods(root_module, cls):
    ## packet.h (module 'network'): ns3::ByteTagIterator::Item::Item(ns3::ByteTagIterator::Item const & arg0) [constructor]
    cls.add_constructor([param('ns3::ByteTagIterator::Item const &', 'arg0')])
    ## packet.h (module 'network'): uint32_t ns3::ByteTagIterator::Item::GetEnd() const [member function]
    cls.add_method('GetEnd', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## packet.h (module 'network'): uint32_t ns3::ByteTagIterator::Item::GetStart() const [member function]
    cls.add_method('GetStart', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## packet.h (module 'network'): void ns3::ByteTagIterator::Item::GetTag(ns3::Tag & tag) const [member function]
    cls.add_method('GetTag', 
                   'void', 
                   [param('ns3::Tag &', 'tag')], 
                   is_const=True)
    ## packet.h (module 'network'): ns3::TypeId ns3::ByteTagIterator::Item::GetTypeId() const [member function]
    cls.add_method('GetTypeId', 
                   'ns3::TypeId', 
                   [], 
                   is_const=True)
    return

def register_Ns3ByteTagList_methods(root_module, cls):
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::ByteTagList() [constructor]
    cls.add_constructor([])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::ByteTagList(ns3::ByteTagList const & o) [constructor]
    cls.add_constructor([param('ns3::ByteTagList const &', 'o')])
    ## byte-tag-list.h (module 'network'): ns3::TagBuffer ns3::ByteTagList::Add(ns3::TypeId tid, uint32_t bufferSize, int32_t start, int32_t end) [member function]
    cls.add_method('Add', 
                   'ns3::TagBuffer', 
                   [param('ns3::TypeId', 'tid'), param('__uint32_t', 'bufferSize'), param('__int32_t', 'start'), param('__int32_t', 'end')])
    ## byte-tag-list.h (module 'network'): void ns3::ByteTagList::Add(ns3::ByteTagList const & o) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::ByteTagList const &', 'o')])
    ## byte-tag-list.h (module 'network'): void ns3::ByteTagList::AddAtEnd(int32_t appendOffset) [member function]
    cls.add_method('AddAtEnd', 
                   'void', 
                   [param('__int32_t', 'appendOffset')])
    ## byte-tag-list.h (module 'network'): void ns3::ByteTagList::AddAtStart(int32_t prependOffset) [member function]
    cls.add_method('AddAtStart', 
                   'void', 
                   [param('__int32_t', 'prependOffset')])
    ## byte-tag-list.h (module 'network'): void ns3::ByteTagList::Adjust(int32_t adjustment) [member function]
    cls.add_method('Adjust', 
                   'void', 
                   [param('__int32_t', 'adjustment')])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator ns3::ByteTagList::Begin(int32_t offsetStart, int32_t offsetEnd) const [member function]
    cls.add_method('Begin', 
                   'ns3::ByteTagList::Iterator', 
                   [param('__int32_t', 'offsetStart'), param('__int32_t', 'offsetEnd')], 
                   is_const=True)
    ## byte-tag-list.h (module 'network'): void ns3::ByteTagList::RemoveAll() [member function]
    cls.add_method('RemoveAll', 
                   'void', 
                   [])
    return

def register_Ns3ByteTagListIterator_methods(root_module, cls):
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Iterator(ns3::ByteTagList::Iterator const & arg0) [constructor]
    cls.add_constructor([param('ns3::ByteTagList::Iterator const &', 'arg0')])
    ## byte-tag-list.h (module 'network'): uint32_t ns3::ByteTagList::Iterator::GetOffsetStart() const [member function]
    cls.add_method('GetOffsetStart', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## byte-tag-list.h (module 'network'): bool ns3::ByteTagList::Iterator::HasNext() const [member function]
    cls.add_method('HasNext', 
                   'bool', 
                   [], 
                   is_const=True)
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item ns3::ByteTagList::Iterator::Next() [member function]
    cls.add_method('Next', 
                   'ns3::ByteTagList::Iterator::Item', 
                   [])
    return

def register_Ns3ByteTagListIteratorItem_methods(root_module, cls):
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::Item(ns3::ByteTagList::Iterator::Item const & arg0) [constructor]
    cls.add_constructor([param('ns3::ByteTagList::Iterator::Item const &', 'arg0')])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::Item(ns3::TagBuffer buf) [constructor]
    cls.add_constructor([param('ns3::TagBuffer', 'buf')])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::buf [variable]
    cls.add_instance_attribute('buf', 'ns3::TagBuffer', is_const=False)
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::end [variable]
    cls.add_instance_attribute('end', 'int32_t', is_const=False)
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::size [variable]
    cls.add_instance_attribute('size', 'uint32_t', is_const=False)
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::start [variable]
    cls.add_instance_attribute('start', 'int32_t', is_const=False)
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item::tid [variable]
    cls.add_instance_attribute('tid', 'ns3::TypeId', is_const=False)
    return

def register_Ns3CallbackBase_methods(root_module, cls):
    ## callback.h (module 'core'): ns3::CallbackBase::CallbackBase(ns3::CallbackBase const & arg0) [constructor]
    cls.add_constructor([param('ns3::CallbackBase const &', 'arg0')])
    ## callback.h (module 'core'): ns3::CallbackBase::CallbackBase() [constructor]
    cls.add_constructor([])
    ## callback.h (module 'core'): ns3::Ptr<ns3::CallbackImplBase> ns3::CallbackBase::GetImpl() const [member function]
    cls.add_method('GetImpl', 
                   'ns3::Ptr< ns3::CallbackImplBase >', 
                   [], 
                   is_const=True)
    ## callback.h (module 'core'): ns3::CallbackBase::CallbackBase(ns3::Ptr<ns3::CallbackImplBase> impl) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::CallbackImplBase >', 'impl')], 
                        visibility='protected')
    return

def register_Ns3DataRate_methods(root_module, cls):
    cls.add_output_stream_operator()
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    cls.add_binary_comparison_operator('<=')
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('>')
    cls.add_binary_comparison_operator('>=')
    ## data-rate.h (module 'network'): ns3::DataRate::DataRate(ns3::DataRate const & arg0) [constructor]
    cls.add_constructor([param('ns3::DataRate const &', 'arg0')])
    ## data-rate.h (module 'network'): ns3::DataRate::DataRate() [constructor]
    cls.add_constructor([])
    ## data-rate.h (module 'network'): ns3::DataRate::DataRate(uint64_t bps) [constructor]
    cls.add_constructor([param('uint64_t', 'bps')])
    ## data-rate.h (module 'network'): ns3::DataRate::DataRate(std::string rate) [constructor]
    cls.add_constructor([param('std::string', 'rate')])
    ## data-rate.h (module 'network'): ns3::Time ns3::DataRate::CalculateBitsTxTime(uint32_t bits) const [member function]
    cls.add_method('CalculateBitsTxTime', 
                   'ns3::Time', 
                   [param('__uint32_t', 'bits')], 
                   is_const=True)
    ## data-rate.h (module 'network'): ns3::Time ns3::DataRate::CalculateBytesTxTime(uint32_t bytes) const [member function]
    cls.add_method('CalculateBytesTxTime', 
                   'ns3::Time', 
                   [param('__uint32_t', 'bytes')], 
                   is_const=True)
    ## data-rate.h (module 'network'): double ns3::DataRate::CalculateTxTime(uint32_t bytes) const [member function]
    cls.add_method('CalculateTxTime', 
                   'double', 
                   [param('__uint32_t', 'bytes')], 
                   deprecated=True, is_const=True)
    ## data-rate.h (module 'network'): uint64_t ns3::DataRate::GetBitRate() const [member function]
    cls.add_method('GetBitRate', 
                   'uint64_t', 
                   [], 
                   is_const=True)
    return

def register_Ns3DefaultDeleter__Ns3AttributeAccessor_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeAccessor>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeAccessor>::DefaultDeleter(ns3::DefaultDeleter<ns3::AttributeAccessor> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::AttributeAccessor > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::AttributeAccessor>::Delete(ns3::AttributeAccessor * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::AttributeAccessor *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3AttributeChecker_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeChecker>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeChecker>::DefaultDeleter(ns3::DefaultDeleter<ns3::AttributeChecker> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::AttributeChecker > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::AttributeChecker>::Delete(ns3::AttributeChecker * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::AttributeChecker *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3AttributeValue_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeValue>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeValue>::DefaultDeleter(ns3::DefaultDeleter<ns3::AttributeValue> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::AttributeValue > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::AttributeValue>::Delete(ns3::AttributeValue * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::AttributeValue *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3CallbackImplBase_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::CallbackImplBase>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::CallbackImplBase>::DefaultDeleter(ns3::DefaultDeleter<ns3::CallbackImplBase> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::CallbackImplBase > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::CallbackImplBase>::Delete(ns3::CallbackImplBase * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::CallbackImplBase *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3EventImpl_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::EventImpl>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::EventImpl>::DefaultDeleter(ns3::DefaultDeleter<ns3::EventImpl> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::EventImpl > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::EventImpl>::Delete(ns3::EventImpl * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::EventImpl *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3HashImplementation_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Hash::Implementation>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Hash::Implementation>::DefaultDeleter(ns3::DefaultDeleter<ns3::Hash::Implementation> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::Hash::Implementation > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::Hash::Implementation>::Delete(ns3::Hash::Implementation * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::Hash::Implementation *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3LoraInterferenceHelperEvent_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event>::DefaultDeleter(ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::LoraInterferenceHelper::Event > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::LoraInterferenceHelper::Event>::Delete(ns3::LoraInterferenceHelper::Event * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::LoraInterferenceHelper::Event *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3NixVector_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::NixVector>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::NixVector>::DefaultDeleter(ns3::DefaultDeleter<ns3::NixVector> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::NixVector > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::NixVector>::Delete(ns3::NixVector * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::NixVector *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3OutputStreamWrapper_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::OutputStreamWrapper>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::OutputStreamWrapper>::DefaultDeleter(ns3::DefaultDeleter<ns3::OutputStreamWrapper> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::OutputStreamWrapper > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::OutputStreamWrapper>::Delete(ns3::OutputStreamWrapper * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::OutputStreamWrapper *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3Packet_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Packet>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Packet>::DefaultDeleter(ns3::DefaultDeleter<ns3::Packet> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::Packet > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::Packet>::Delete(ns3::Packet * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::Packet *', 'object')], 
                   is_static=True)
    return

def register_Ns3DefaultDeleter__Ns3TraceSourceAccessor_methods(root_module, cls):
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::TraceSourceAccessor>::DefaultDeleter() [constructor]
    cls.add_constructor([])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::TraceSourceAccessor>::DefaultDeleter(ns3::DefaultDeleter<ns3::TraceSourceAccessor> const & arg0) [constructor]
    cls.add_constructor([param('ns3::DefaultDeleter< ns3::TraceSourceAccessor > const &', 'arg0')])
    ## default-deleter.h (module 'core'): static void ns3::DefaultDeleter<ns3::TraceSourceAccessor>::Delete(ns3::TraceSourceAccessor * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::TraceSourceAccessor *', 'object')], 
                   is_static=True)
    return

def register_Ns3DeviceStatus_methods(root_module, cls):
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::DeviceStatus(ns3::DeviceStatus const & arg0) [constructor]
    cls.add_constructor([param('ns3::DeviceStatus const &', 'arg0')])
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::DeviceStatus() [constructor]
    cls.add_constructor([])
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::DeviceStatus(ns3::Ptr<ns3::EndDeviceLoraMac> endDeviceMac) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::EndDeviceLoraMac >', 'endDeviceMac')])
    ## device-status.h (module 'lorawan'): ns3::LoraDeviceAddress ns3::DeviceStatus::GetAddress() [member function]
    cls.add_method('GetAddress', 
                   'ns3::LoraDeviceAddress', 
                   [])
    ## device-status.h (module 'lorawan'): ns3::Address ns3::DeviceStatus::GetBestGatewayAddress() [member function]
    cls.add_method('GetBestGatewayAddress', 
                   'ns3::Address', 
                   [])
    ## device-status.h (module 'lorawan'): uint8_t ns3::DeviceStatus::GetDataRate() [member function]
    cls.add_method('GetDataRate', 
                   'uint8_t', 
                   [])
    ## device-status.h (module 'lorawan'): uint8_t ns3::DeviceStatus::GetFirstReceiveWindowDataRate() [member function]
    cls.add_method('GetFirstReceiveWindowDataRate', 
                   'uint8_t', 
                   [])
    ## device-status.h (module 'lorawan'): double ns3::DeviceStatus::GetFirstReceiveWindowFrequency() [member function]
    cls.add_method('GetFirstReceiveWindowFrequency', 
                   'double', 
                   [])
    ## device-status.h (module 'lorawan'): ns3::Ptr<ns3::Packet> ns3::DeviceStatus::GetReplyPacket() [member function]
    cls.add_method('GetReplyPacket', 
                   'ns3::Ptr< ns3::Packet >', 
                   [])
    ## device-status.h (module 'lorawan'): uint8_t ns3::DeviceStatus::GetSecondReceiveWindowDataRate() [member function]
    cls.add_method('GetSecondReceiveWindowDataRate', 
                   'uint8_t', 
                   [])
    ## device-status.h (module 'lorawan'): double ns3::DeviceStatus::GetSecondReceiveWindowFrequency() [member function]
    cls.add_method('GetSecondReceiveWindowFrequency', 
                   'double', 
                   [])
    ## device-status.h (module 'lorawan'): std::list<ns3::Address, std::allocator<ns3::Address> > ns3::DeviceStatus::GetSortedGatewayAddresses() [member function]
    cls.add_method('GetSortedGatewayAddresses', 
                   'std::list< ns3::Address >', 
                   [])
    ## device-status.h (module 'lorawan'): bool ns3::DeviceStatus::HasReply() [member function]
    cls.add_method('HasReply', 
                   'bool', 
                   [])
    ## device-status.h (module 'lorawan'): void ns3::DeviceStatus::SetAddress(ns3::LoraDeviceAddress address) [member function]
    cls.add_method('SetAddress', 
                   'void', 
                   [param('ns3::LoraDeviceAddress', 'address')])
    ## device-status.h (module 'lorawan'): void ns3::DeviceStatus::SetFirstReceiveWindowFrequency(double frequency) [member function]
    cls.add_method('SetFirstReceiveWindowFrequency', 
                   'void', 
                   [param('double', 'frequency')])
    ## device-status.h (module 'lorawan'): void ns3::DeviceStatus::SetReply(ns3::DeviceStatus::Reply reply) [member function]
    cls.add_method('SetReply', 
                   'void', 
                   [param('ns3::DeviceStatus::Reply', 'reply')])
    ## device-status.h (module 'lorawan'): void ns3::DeviceStatus::UpdateGatewayData(ns3::Address gwAddress, double rcvPower) [member function]
    cls.add_method('UpdateGatewayData', 
                   'void', 
                   [param('ns3::Address', 'gwAddress'), param('double', 'rcvPower')])
    return

def register_Ns3DeviceStatusReply_methods(root_module, cls):
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply::Reply() [constructor]
    cls.add_constructor([])
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply::Reply(ns3::DeviceStatus::Reply const & arg0) [constructor]
    cls.add_constructor([param('ns3::DeviceStatus::Reply const &', 'arg0')])
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply::frameHeader [variable]
    cls.add_instance_attribute('frameHeader', 'ns3::LoraFrameHeader', is_const=False)
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply::hasReply [variable]
    cls.add_instance_attribute('hasReply', 'bool', is_const=False)
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply::macHeader [variable]
    cls.add_instance_attribute('macHeader', 'ns3::LoraMacHeader', is_const=False)
    ## device-status.h (module 'lorawan'): ns3::DeviceStatus::Reply::packet [variable]
    cls.add_instance_attribute('packet', 'ns3::Ptr< ns3::Packet >', is_const=False)
    return

def register_Ns3EventId_methods(root_module, cls):
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    ## event-id.h (module 'core'): ns3::EventId::EventId(ns3::EventId const & arg0) [constructor]
    cls.add_constructor([param('ns3::EventId const &', 'arg0')])
    ## event-id.h (module 'core'): ns3::EventId::EventId() [constructor]
    cls.add_constructor([])
    ## event-id.h (module 'core'): ns3::EventId::EventId(ns3::Ptr<ns3::EventImpl> const & impl, uint64_t ts, uint32_t context, uint32_t uid) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::EventImpl > const &', 'impl'), param('uint64_t', 'ts'), param('uint32_t', 'context'), param('uint32_t', 'uid')])
    ## event-id.h (module 'core'): void ns3::EventId::Cancel() [member function]
    cls.add_method('Cancel', 
                   'void', 
                   [])
    ## event-id.h (module 'core'): uint32_t ns3::EventId::GetContext() const [member function]
    cls.add_method('GetContext', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## event-id.h (module 'core'): uint64_t ns3::EventId::GetTs() const [member function]
    cls.add_method('GetTs', 
                   'uint64_t', 
                   [], 
                   is_const=True)
    ## event-id.h (module 'core'): uint32_t ns3::EventId::GetUid() const [member function]
    cls.add_method('GetUid', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## event-id.h (module 'core'): bool ns3::EventId::IsExpired() const [member function]
    cls.add_method('IsExpired', 
                   'bool', 
                   [], 
                   is_const=True)
    ## event-id.h (module 'core'): bool ns3::EventId::IsRunning() const [member function]
    cls.add_method('IsRunning', 
                   'bool', 
                   [], 
                   is_const=True)
    ## event-id.h (module 'core'): ns3::EventImpl * ns3::EventId::PeekEventImpl() const [member function]
    cls.add_method('PeekEventImpl', 
                   'ns3::EventImpl *', 
                   [], 
                   is_const=True)
    return

def register_Ns3ForwarderHelper_methods(root_module, cls):
    ## forwarder-helper.h (module 'lorawan'): ns3::ForwarderHelper::ForwarderHelper(ns3::ForwarderHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::ForwarderHelper const &', 'arg0')])
    ## forwarder-helper.h (module 'lorawan'): ns3::ForwarderHelper::ForwarderHelper() [constructor]
    cls.add_constructor([])
    ## forwarder-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::ForwarderHelper::Install(ns3::NodeContainer c) const [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::NodeContainer', 'c')], 
                   is_const=True)
    ## forwarder-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::ForwarderHelper::Install(ns3::Ptr<ns3::Node> node) const [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::Ptr< ns3::Node >', 'node')], 
                   is_const=True)
    ## forwarder-helper.h (module 'lorawan'): void ns3::ForwarderHelper::SetAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    return

def register_Ns3GatewayStatus_methods(root_module, cls):
    ## gateway-status.h (module 'lorawan'): ns3::GatewayStatus::GatewayStatus(ns3::GatewayStatus const & arg0) [constructor]
    cls.add_constructor([param('ns3::GatewayStatus const &', 'arg0')])
    ## gateway-status.h (module 'lorawan'): ns3::GatewayStatus::GatewayStatus() [constructor]
    cls.add_constructor([])
    ## gateway-status.h (module 'lorawan'): ns3::GatewayStatus::GatewayStatus(ns3::Address address, ns3::Ptr<ns3::NetDevice> netDevice, ns3::Ptr<ns3::GatewayLoraMac> gwMac) [constructor]
    cls.add_constructor([param('ns3::Address', 'address'), param('ns3::Ptr< ns3::NetDevice >', 'netDevice'), param('ns3::Ptr< ns3::GatewayLoraMac >', 'gwMac')])
    ## gateway-status.h (module 'lorawan'): ns3::Address ns3::GatewayStatus::GetAddress() [member function]
    cls.add_method('GetAddress', 
                   'ns3::Address', 
                   [])
    ## gateway-status.h (module 'lorawan'): ns3::Ptr<ns3::GatewayLoraMac> ns3::GatewayStatus::GetGatewayMac() [member function]
    cls.add_method('GetGatewayMac', 
                   'ns3::Ptr< ns3::GatewayLoraMac >', 
                   [])
    ## gateway-status.h (module 'lorawan'): ns3::Ptr<ns3::NetDevice> ns3::GatewayStatus::GetNetDevice() [member function]
    cls.add_method('GetNetDevice', 
                   'ns3::Ptr< ns3::NetDevice >', 
                   [])
    ## gateway-status.h (module 'lorawan'): bool ns3::GatewayStatus::IsAvailableForTransmission(double frequency) [member function]
    cls.add_method('IsAvailableForTransmission', 
                   'bool', 
                   [param('double', 'frequency')])
    ## gateway-status.h (module 'lorawan'): void ns3::GatewayStatus::SetAddress(ns3::Address address) [member function]
    cls.add_method('SetAddress', 
                   'void', 
                   [param('ns3::Address', 'address')])
    ## gateway-status.h (module 'lorawan'): void ns3::GatewayStatus::SetNetDevice(ns3::Ptr<ns3::NetDevice> netDevice) [member function]
    cls.add_method('SetNetDevice', 
                   'void', 
                   [param('ns3::Ptr< ns3::NetDevice >', 'netDevice')])
    ## gateway-status.h (module 'lorawan'): void ns3::GatewayStatus::SetNextTransmissionTime(ns3::Time nextTransmissionTime) [member function]
    cls.add_method('SetNextTransmissionTime', 
                   'void', 
                   [param('ns3::Time', 'nextTransmissionTime')])
    return

def register_Ns3Hasher_methods(root_module, cls):
    ## hash.h (module 'core'): ns3::Hasher::Hasher(ns3::Hasher const & arg0) [constructor]
    cls.add_constructor([param('ns3::Hasher const &', 'arg0')])
    ## hash.h (module 'core'): ns3::Hasher::Hasher() [constructor]
    cls.add_constructor([])
    ## hash.h (module 'core'): ns3::Hasher::Hasher(ns3::Ptr<ns3::Hash::Implementation> hp) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::Hash::Implementation >', 'hp')])
    ## hash.h (module 'core'): uint32_t ns3::Hasher::GetHash32(char const * buffer, size_t const size) [member function]
    cls.add_method('GetHash32', 
                   'uint32_t', 
                   [param('char const *', 'buffer'), param('size_t const', 'size')])
    ## hash.h (module 'core'): uint32_t ns3::Hasher::GetHash32(std::string const s) [member function]
    cls.add_method('GetHash32', 
                   'uint32_t', 
                   [param('std::string const', 's')])
    ## hash.h (module 'core'): uint64_t ns3::Hasher::GetHash64(char const * buffer, size_t const size) [member function]
    cls.add_method('GetHash64', 
                   'uint64_t', 
                   [param('char const *', 'buffer'), param('size_t const', 'size')])
    ## hash.h (module 'core'): uint64_t ns3::Hasher::GetHash64(std::string const s) [member function]
    cls.add_method('GetHash64', 
                   'uint64_t', 
                   [param('std::string const', 's')])
    ## hash.h (module 'core'): ns3::Hasher & ns3::Hasher::clear() [member function]
    cls.add_method('clear', 
                   'ns3::Hasher &', 
                   [])
    return

def register_Ns3Ipv4Address_methods(root_module, cls):
    cls.add_output_stream_operator()
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address::Ipv4Address(ns3::Ipv4Address const & arg0) [constructor]
    cls.add_constructor([param('ns3::Ipv4Address const &', 'arg0')])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address::Ipv4Address() [constructor]
    cls.add_constructor([])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address::Ipv4Address(uint32_t address) [constructor]
    cls.add_constructor([param('uint32_t', 'address')])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address::Ipv4Address(char const * address) [constructor]
    cls.add_constructor([param('char const *', 'address')])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address ns3::Ipv4Address::CombineMask(ns3::Ipv4Mask const & mask) const [member function]
    cls.add_method('CombineMask', 
                   'ns3::Ipv4Address', 
                   [param('ns3::Ipv4Mask const &', 'mask')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Address ns3::Ipv4Address::ConvertFrom(ns3::Address const & address) [member function]
    cls.add_method('ConvertFrom', 
                   'ns3::Ipv4Address', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Address ns3::Ipv4Address::Deserialize(uint8_t const * buf) [member function]
    cls.add_method('Deserialize', 
                   'ns3::Ipv4Address', 
                   [param('uint8_t const *', 'buf')], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): uint32_t ns3::Ipv4Address::Get() const [member function]
    cls.add_method('Get', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Address ns3::Ipv4Address::GetAny() [member function]
    cls.add_method('GetAny', 
                   'ns3::Ipv4Address', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Address ns3::Ipv4Address::GetBroadcast() [member function]
    cls.add_method('GetBroadcast', 
                   'ns3::Ipv4Address', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Address ns3::Ipv4Address::GetLoopback() [member function]
    cls.add_method('GetLoopback', 
                   'ns3::Ipv4Address', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address ns3::Ipv4Address::GetSubnetDirectedBroadcast(ns3::Ipv4Mask const & mask) const [member function]
    cls.add_method('GetSubnetDirectedBroadcast', 
                   'ns3::Ipv4Address', 
                   [param('ns3::Ipv4Mask const &', 'mask')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Address ns3::Ipv4Address::GetZero() [member function]
    cls.add_method('GetZero', 
                   'ns3::Ipv4Address', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsAny() const [member function]
    cls.add_method('IsAny', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsBroadcast() const [member function]
    cls.add_method('IsBroadcast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsEqual(ns3::Ipv4Address const & other) const [member function]
    cls.add_method('IsEqual', 
                   'bool', 
                   [param('ns3::Ipv4Address const &', 'other')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsLocalMulticast() const [member function]
    cls.add_method('IsLocalMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsLocalhost() const [member function]
    cls.add_method('IsLocalhost', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): static bool ns3::Ipv4Address::IsMatchingType(ns3::Address const & address) [member function]
    cls.add_method('IsMatchingType', 
                   'bool', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsMulticast() const [member function]
    cls.add_method('IsMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Address::IsSubnetDirectedBroadcast(ns3::Ipv4Mask const & mask) const [member function]
    cls.add_method('IsSubnetDirectedBroadcast', 
                   'bool', 
                   [param('ns3::Ipv4Mask const &', 'mask')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): void ns3::Ipv4Address::Print(std::ostream & os) const [member function]
    cls.add_method('Print', 
                   'void', 
                   [param('std::ostream &', 'os')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): void ns3::Ipv4Address::Serialize(uint8_t * buf) const [member function]
    cls.add_method('Serialize', 
                   'void', 
                   [param('uint8_t *', 'buf')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): void ns3::Ipv4Address::Set(uint32_t address) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('__uint32_t', 'address')])
    ## ipv4-address.h (module 'network'): void ns3::Ipv4Address::Set(char const * address) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('char const *', 'address')])
    return

def register_Ns3Ipv4Mask_methods(root_module, cls):
    cls.add_output_stream_operator()
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Mask::Ipv4Mask(ns3::Ipv4Mask const & arg0) [constructor]
    cls.add_constructor([param('ns3::Ipv4Mask const &', 'arg0')])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Mask::Ipv4Mask() [constructor]
    cls.add_constructor([])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Mask::Ipv4Mask(uint32_t mask) [constructor]
    cls.add_constructor([param('uint32_t', 'mask')])
    ## ipv4-address.h (module 'network'): ns3::Ipv4Mask::Ipv4Mask(char const * mask) [constructor]
    cls.add_constructor([param('char const *', 'mask')])
    ## ipv4-address.h (module 'network'): uint32_t ns3::Ipv4Mask::Get() const [member function]
    cls.add_method('Get', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): uint32_t ns3::Ipv4Mask::GetInverse() const [member function]
    cls.add_method('GetInverse', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Mask ns3::Ipv4Mask::GetLoopback() [member function]
    cls.add_method('GetLoopback', 
                   'ns3::Ipv4Mask', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Mask ns3::Ipv4Mask::GetOnes() [member function]
    cls.add_method('GetOnes', 
                   'ns3::Ipv4Mask', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): uint16_t ns3::Ipv4Mask::GetPrefixLength() const [member function]
    cls.add_method('GetPrefixLength', 
                   'uint16_t', 
                   [], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): static ns3::Ipv4Mask ns3::Ipv4Mask::GetZero() [member function]
    cls.add_method('GetZero', 
                   'ns3::Ipv4Mask', 
                   [], 
                   is_static=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Mask::IsEqual(ns3::Ipv4Mask other) const [member function]
    cls.add_method('IsEqual', 
                   'bool', 
                   [param('ns3::Ipv4Mask', 'other')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): bool ns3::Ipv4Mask::IsMatch(ns3::Ipv4Address a, ns3::Ipv4Address b) const [member function]
    cls.add_method('IsMatch', 
                   'bool', 
                   [param('ns3::Ipv4Address', 'a'), param('ns3::Ipv4Address', 'b')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): void ns3::Ipv4Mask::Print(std::ostream & os) const [member function]
    cls.add_method('Print', 
                   'void', 
                   [param('std::ostream &', 'os')], 
                   is_const=True)
    ## ipv4-address.h (module 'network'): void ns3::Ipv4Mask::Set(uint32_t mask) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('__uint32_t', 'mask')])
    return

def register_Ns3Ipv6Address_methods(root_module, cls):
    cls.add_output_stream_operator()
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address::Ipv6Address() [constructor]
    cls.add_constructor([])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address::Ipv6Address(char const * address) [constructor]
    cls.add_constructor([param('char const *', 'address')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address::Ipv6Address(uint8_t * address) [constructor]
    cls.add_constructor([param('uint8_t *', 'address')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address::Ipv6Address(ns3::Ipv6Address const & addr) [constructor]
    cls.add_constructor([param('ns3::Ipv6Address const &', 'addr')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address::Ipv6Address(ns3::Ipv6Address const * addr) [constructor]
    cls.add_constructor([param('ns3::Ipv6Address const *', 'addr')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address ns3::Ipv6Address::CombinePrefix(ns3::Ipv6Prefix const & prefix) [member function]
    cls.add_method('CombinePrefix', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Ipv6Prefix const &', 'prefix')])
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::ConvertFrom(ns3::Address const & address) [member function]
    cls.add_method('ConvertFrom', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::Deserialize(uint8_t const * buf) [member function]
    cls.add_method('Deserialize', 
                   'ns3::Ipv6Address', 
                   [param('uint8_t const *', 'buf')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetAllHostsMulticast() [member function]
    cls.add_method('GetAllHostsMulticast', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetAllNodesMulticast() [member function]
    cls.add_method('GetAllNodesMulticast', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetAllRoutersMulticast() [member function]
    cls.add_method('GetAllRoutersMulticast', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetAny() [member function]
    cls.add_method('GetAny', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Address::GetBytes(uint8_t * buf) const [member function]
    cls.add_method('GetBytes', 
                   'void', 
                   [param('uint8_t *', 'buf')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): ns3::Ipv4Address ns3::Ipv6Address::GetIpv4MappedAddress() const [member function]
    cls.add_method('GetIpv4MappedAddress', 
                   'ns3::Ipv4Address', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetLoopback() [member function]
    cls.add_method('GetLoopback', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetOnes() [member function]
    cls.add_method('GetOnes', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::GetZero() [member function]
    cls.add_method('GetZero', 
                   'ns3::Ipv6Address', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsAllHostsMulticast() const [member function]
    cls.add_method('IsAllHostsMulticast', 
                   'bool', 
                   [], 
                   deprecated=True, is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsAllNodesMulticast() const [member function]
    cls.add_method('IsAllNodesMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsAllRoutersMulticast() const [member function]
    cls.add_method('IsAllRoutersMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsAny() const [member function]
    cls.add_method('IsAny', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsDocumentation() const [member function]
    cls.add_method('IsDocumentation', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsEqual(ns3::Ipv6Address const & other) const [member function]
    cls.add_method('IsEqual', 
                   'bool', 
                   [param('ns3::Ipv6Address const &', 'other')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsIpv4MappedAddress() const [member function]
    cls.add_method('IsIpv4MappedAddress', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsLinkLocal() const [member function]
    cls.add_method('IsLinkLocal', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsLinkLocalMulticast() const [member function]
    cls.add_method('IsLinkLocalMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsLocalhost() const [member function]
    cls.add_method('IsLocalhost', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): static bool ns3::Ipv6Address::IsMatchingType(ns3::Address const & address) [member function]
    cls.add_method('IsMatchingType', 
                   'bool', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsMulticast() const [member function]
    cls.add_method('IsMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Address::IsSolicitedMulticast() const [member function]
    cls.add_method('IsSolicitedMulticast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeAutoconfiguredAddress(ns3::Mac16Address addr, ns3::Ipv6Address prefix) [member function]
    cls.add_method('MakeAutoconfiguredAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Mac16Address', 'addr'), param('ns3::Ipv6Address', 'prefix')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeAutoconfiguredAddress(ns3::Mac48Address addr, ns3::Ipv6Address prefix) [member function]
    cls.add_method('MakeAutoconfiguredAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Mac48Address', 'addr'), param('ns3::Ipv6Address', 'prefix')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeAutoconfiguredAddress(ns3::Mac64Address addr, ns3::Ipv6Address prefix) [member function]
    cls.add_method('MakeAutoconfiguredAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Mac64Address', 'addr'), param('ns3::Ipv6Address', 'prefix')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeAutoconfiguredLinkLocalAddress(ns3::Mac16Address mac) [member function]
    cls.add_method('MakeAutoconfiguredLinkLocalAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Mac16Address', 'mac')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeAutoconfiguredLinkLocalAddress(ns3::Mac48Address mac) [member function]
    cls.add_method('MakeAutoconfiguredLinkLocalAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Mac48Address', 'mac')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeAutoconfiguredLinkLocalAddress(ns3::Mac64Address mac) [member function]
    cls.add_method('MakeAutoconfiguredLinkLocalAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Mac64Address', 'mac')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeIpv4MappedAddress(ns3::Ipv4Address addr) [member function]
    cls.add_method('MakeIpv4MappedAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Ipv4Address', 'addr')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Address ns3::Ipv6Address::MakeSolicitedAddress(ns3::Ipv6Address addr) [member function]
    cls.add_method('MakeSolicitedAddress', 
                   'ns3::Ipv6Address', 
                   [param('ns3::Ipv6Address', 'addr')], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Address::Print(std::ostream & os) const [member function]
    cls.add_method('Print', 
                   'void', 
                   [param('std::ostream &', 'os')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Address::Serialize(uint8_t * buf) const [member function]
    cls.add_method('Serialize', 
                   'void', 
                   [param('uint8_t *', 'buf')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Address::Set(char const * address) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('char const *', 'address')])
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Address::Set(uint8_t * address) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('uint8_t *', 'address')])
    return

def register_Ns3Ipv6Prefix_methods(root_module, cls):
    cls.add_output_stream_operator()
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix::Ipv6Prefix() [constructor]
    cls.add_constructor([])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix::Ipv6Prefix(uint8_t * prefix) [constructor]
    cls.add_constructor([param('uint8_t *', 'prefix')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix::Ipv6Prefix(char const * prefix) [constructor]
    cls.add_constructor([param('char const *', 'prefix')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix::Ipv6Prefix(uint8_t prefix) [constructor]
    cls.add_constructor([param('uint8_t', 'prefix')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix::Ipv6Prefix(ns3::Ipv6Prefix const & prefix) [constructor]
    cls.add_constructor([param('ns3::Ipv6Prefix const &', 'prefix')])
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix::Ipv6Prefix(ns3::Ipv6Prefix const * prefix) [constructor]
    cls.add_constructor([param('ns3::Ipv6Prefix const *', 'prefix')])
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Prefix::GetBytes(uint8_t * buf) const [member function]
    cls.add_method('GetBytes', 
                   'void', 
                   [param('uint8_t *', 'buf')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Prefix ns3::Ipv6Prefix::GetLoopback() [member function]
    cls.add_method('GetLoopback', 
                   'ns3::Ipv6Prefix', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Prefix ns3::Ipv6Prefix::GetOnes() [member function]
    cls.add_method('GetOnes', 
                   'ns3::Ipv6Prefix', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): uint8_t ns3::Ipv6Prefix::GetPrefixLength() const [member function]
    cls.add_method('GetPrefixLength', 
                   'uint8_t', 
                   [], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): static ns3::Ipv6Prefix ns3::Ipv6Prefix::GetZero() [member function]
    cls.add_method('GetZero', 
                   'ns3::Ipv6Prefix', 
                   [], 
                   is_static=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Prefix::IsEqual(ns3::Ipv6Prefix const & other) const [member function]
    cls.add_method('IsEqual', 
                   'bool', 
                   [param('ns3::Ipv6Prefix const &', 'other')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): bool ns3::Ipv6Prefix::IsMatch(ns3::Ipv6Address a, ns3::Ipv6Address b) const [member function]
    cls.add_method('IsMatch', 
                   'bool', 
                   [param('ns3::Ipv6Address', 'a'), param('ns3::Ipv6Address', 'b')], 
                   is_const=True)
    ## ipv6-address.h (module 'network'): void ns3::Ipv6Prefix::Print(std::ostream & os) const [member function]
    cls.add_method('Print', 
                   'void', 
                   [param('std::ostream &', 'os')], 
                   is_const=True)
    return

def register_Ns3LoraChannelParameters_methods(root_module, cls):
    cls.add_output_stream_operator()
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters::LoraChannelParameters() [constructor]
    cls.add_constructor([])
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters::LoraChannelParameters(ns3::LoraChannelParameters const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraChannelParameters const &', 'arg0')])
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters::duration [variable]
    cls.add_instance_attribute('duration', 'ns3::Time', is_const=False)
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters::frequencyMHz [variable]
    cls.add_instance_attribute('frequencyMHz', 'double', is_const=False)
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters::rxPowerDbm [variable]
    cls.add_instance_attribute('rxPowerDbm', 'double', is_const=False)
    ## lora-channel.h (module 'lorawan'): ns3::LoraChannelParameters::sf [variable]
    cls.add_instance_attribute('sf', 'uint8_t', is_const=False)
    return

def register_Ns3LoraDeviceAddress_methods(root_module, cls):
    cls.add_output_stream_operator()
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('>')
    ## lora-device-address.h (module 'lorawan'): ns3::LoraDeviceAddress::LoraDeviceAddress(ns3::LoraDeviceAddress const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraDeviceAddress const &', 'arg0')])
    ## lora-device-address.h (module 'lorawan'): ns3::LoraDeviceAddress::LoraDeviceAddress() [constructor]
    cls.add_constructor([])
    ## lora-device-address.h (module 'lorawan'): ns3::LoraDeviceAddress::LoraDeviceAddress(uint32_t address) [constructor]
    cls.add_constructor([param('uint32_t', 'address')])
    ## lora-device-address.h (module 'lorawan'): ns3::LoraDeviceAddress::LoraDeviceAddress(uint8_t nwkId, uint32_t nwkAddr) [constructor]
    cls.add_constructor([param('uint8_t', 'nwkId'), param('uint32_t', 'nwkAddr')])
    ## lora-device-address.h (module 'lorawan'): ns3::LoraDeviceAddress::LoraDeviceAddress(ns3::NwkID nwkId, ns3::NwkAddr nwkAddr) [constructor]
    cls.add_constructor([param('ns3::NwkID', 'nwkId'), param('ns3::NwkAddr', 'nwkAddr')])
    ## lora-device-address.h (module 'lorawan'): static ns3::LoraDeviceAddress ns3::LoraDeviceAddress::ConvertFrom(ns3::Address const & address) [member function]
    cls.add_method('ConvertFrom', 
                   'ns3::LoraDeviceAddress', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    ## lora-device-address.h (module 'lorawan'): static ns3::LoraDeviceAddress ns3::LoraDeviceAddress::Deserialize(uint8_t const * buf) [member function]
    cls.add_method('Deserialize', 
                   'ns3::LoraDeviceAddress', 
                   [param('uint8_t const *', 'buf')], 
                   is_static=True)
    ## lora-device-address.h (module 'lorawan'): uint32_t ns3::LoraDeviceAddress::Get() const [member function]
    cls.add_method('Get', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## lora-device-address.h (module 'lorawan'): uint32_t ns3::LoraDeviceAddress::GetNwkAddr() [member function]
    cls.add_method('GetNwkAddr', 
                   'uint32_t', 
                   [])
    ## lora-device-address.h (module 'lorawan'): uint8_t ns3::LoraDeviceAddress::GetNwkID() [member function]
    cls.add_method('GetNwkID', 
                   'uint8_t', 
                   [])
    ## lora-device-address.h (module 'lorawan'): std::string ns3::LoraDeviceAddress::Print() const [member function]
    cls.add_method('Print', 
                   'std::string', 
                   [], 
                   is_const=True)
    ## lora-device-address.h (module 'lorawan'): void ns3::LoraDeviceAddress::Serialize(uint8_t * buf) const [member function]
    cls.add_method('Serialize', 
                   'void', 
                   [param('uint8_t *', 'buf')], 
                   is_const=True)
    ## lora-device-address.h (module 'lorawan'): void ns3::LoraDeviceAddress::Set(uint32_t address) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('__uint32_t', 'address')])
    ## lora-device-address.h (module 'lorawan'): void ns3::LoraDeviceAddress::Set(uint8_t nwkId, uint32_t nwkAddr) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('__uint8_t', 'nwkId'), param('__uint32_t', 'nwkAddr')])
    ## lora-device-address.h (module 'lorawan'): void ns3::LoraDeviceAddress::SetNwkAddr(uint32_t nwkAddr) [member function]
    cls.add_method('SetNwkAddr', 
                   'void', 
                   [param('__uint32_t', 'nwkAddr')])
    ## lora-device-address.h (module 'lorawan'): void ns3::LoraDeviceAddress::SetNwkID(uint8_t nwkId) [member function]
    cls.add_method('SetNwkID', 
                   'void', 
                   [param('__uint8_t', 'nwkId')])
    return

def register_Ns3LoraHelper_methods(root_module, cls):
    ## lora-helper.h (module 'lorawan'): ns3::LoraHelper::LoraHelper(ns3::LoraHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraHelper const &', 'arg0')])
    ## lora-helper.h (module 'lorawan'): ns3::LoraHelper::LoraHelper() [constructor]
    cls.add_constructor([])
    ## lora-helper.h (module 'lorawan'): ns3::NetDeviceContainer ns3::LoraHelper::Install(ns3::LoraPhyHelper const & phyHelper, ns3::LoraMacHelper const & macHelper, ns3::NodeContainer c) const [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('ns3::LoraPhyHelper const &', 'phyHelper'), param('ns3::LoraMacHelper const &', 'macHelper'), param('ns3::NodeContainer', 'c')], 
                   is_const=True, is_virtual=True)
    ## lora-helper.h (module 'lorawan'): ns3::NetDeviceContainer ns3::LoraHelper::Install(ns3::LoraPhyHelper const & phyHelper, ns3::LoraMacHelper const & macHelper, ns3::Ptr<ns3::Node> node) const [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('ns3::LoraPhyHelper const &', 'phyHelper'), param('ns3::LoraMacHelper const &', 'macHelper'), param('ns3::Ptr< ns3::Node >', 'node')], 
                   is_const=True, is_virtual=True)
    return

def register_Ns3LoraInterferenceHelper_methods(root_module, cls):
    ## lora-interference-helper.h (module 'lorawan'): ns3::LoraInterferenceHelper::LoraInterferenceHelper(ns3::LoraInterferenceHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraInterferenceHelper const &', 'arg0')])
    ## lora-interference-helper.h (module 'lorawan'): ns3::LoraInterferenceHelper::LoraInterferenceHelper() [constructor]
    cls.add_constructor([])
    ## lora-interference-helper.h (module 'lorawan'): ns3::Ptr<ns3::LoraInterferenceHelper::Event> ns3::LoraInterferenceHelper::Add(ns3::Time duration, double rxPower, uint8_t spreadingFactor, ns3::Ptr<ns3::Packet> packet, double frequencyMHz) [member function]
    cls.add_method('Add', 
                   'ns3::Ptr< ns3::LoraInterferenceHelper::Event >', 
                   [param('ns3::Time', 'duration'), param('double', 'rxPower'), param('__uint8_t', 'spreadingFactor'), param('ns3::Ptr< ns3::Packet >', 'packet'), param('double', 'frequencyMHz')])
    ## lora-interference-helper.h (module 'lorawan'): void ns3::LoraInterferenceHelper::CleanOldEvents() [member function]
    cls.add_method('CleanOldEvents', 
                   'void', 
                   [])
    ## lora-interference-helper.h (module 'lorawan'): void ns3::LoraInterferenceHelper::ClearAllEvents() [member function]
    cls.add_method('ClearAllEvents', 
                   'void', 
                   [])
    ## lora-interference-helper.h (module 'lorawan'): std::list<ns3::Ptr<ns3::LoraInterferenceHelper::Event>, std::allocator<ns3::Ptr<ns3::LoraInterferenceHelper::Event> > > ns3::LoraInterferenceHelper::GetInterferers() [member function]
    cls.add_method('GetInterferers', 
                   'std::list< ns3::Ptr< ns3::LoraInterferenceHelper::Event > >', 
                   [])
    ## lora-interference-helper.h (module 'lorawan'): ns3::Time ns3::LoraInterferenceHelper::GetOverlapTime(ns3::Ptr<ns3::LoraInterferenceHelper::Event> event1, ns3::Ptr<ns3::LoraInterferenceHelper::Event> event2) [member function]
    cls.add_method('GetOverlapTime', 
                   'ns3::Time', 
                   [param('ns3::Ptr< ns3::LoraInterferenceHelper::Event >', 'event1'), param('ns3::Ptr< ns3::LoraInterferenceHelper::Event >', 'event2')])
    ## lora-interference-helper.h (module 'lorawan'): static ns3::TypeId ns3::LoraInterferenceHelper::GetTypeId() [member function]
    cls.add_method('GetTypeId', 
                   'ns3::TypeId', 
                   [], 
                   is_static=True)
    ## lora-interference-helper.h (module 'lorawan'): uint8_t ns3::LoraInterferenceHelper::IsDestroyedByInterference(ns3::Ptr<ns3::LoraInterferenceHelper::Event> event) [member function]
    cls.add_method('IsDestroyedByInterference', 
                   'uint8_t', 
                   [param('ns3::Ptr< ns3::LoraInterferenceHelper::Event >', 'event')])
    ## lora-interference-helper.h (module 'lorawan'): void ns3::LoraInterferenceHelper::PrintEvents(std::ostream & stream) [member function]
    cls.add_method('PrintEvents', 
                   'void', 
                   [param('std::ostream &', 'stream')])
    return

def register_Ns3LoraMacHelper_methods(root_module, cls):
    ## lora-mac-helper.h (module 'lorawan'): ns3::LoraMacHelper::LoraMacHelper(ns3::LoraMacHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraMacHelper const &', 'arg0')])
    ## lora-mac-helper.h (module 'lorawan'): ns3::LoraMacHelper::LoraMacHelper() [constructor]
    cls.add_constructor([])
    ## lora-mac-helper.h (module 'lorawan'): ns3::Ptr<ns3::LoraMac> ns3::LoraMacHelper::Create(ns3::Ptr<ns3::Node> node, ns3::Ptr<ns3::NetDevice> device) const [member function]
    cls.add_method('Create', 
                   'ns3::Ptr< ns3::LoraMac >', 
                   [param('ns3::Ptr< ns3::Node >', 'node'), param('ns3::Ptr< ns3::NetDevice >', 'device')], 
                   is_const=True)
    ## lora-mac-helper.h (module 'lorawan'): void ns3::LoraMacHelper::Set(std::string name, ns3::AttributeValue const & v) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'v')])
    ## lora-mac-helper.h (module 'lorawan'): void ns3::LoraMacHelper::SetAddressGenerator(ns3::Ptr<ns3::LoraDeviceAddressGenerator> addrGen) [member function]
    cls.add_method('SetAddressGenerator', 
                   'void', 
                   [param('ns3::Ptr< ns3::LoraDeviceAddressGenerator >', 'addrGen')])
    ## lora-mac-helper.h (module 'lorawan'): void ns3::LoraMacHelper::SetDeviceType(ns3::LoraMacHelper::DeviceType dt) [member function]
    cls.add_method('SetDeviceType', 
                   'void', 
                   [param('ns3::LoraMacHelper::DeviceType', 'dt')])
    ## lora-mac-helper.h (module 'lorawan'): void ns3::LoraMacHelper::SetRegion(ns3::LoraMacHelper::Regions region) [member function]
    cls.add_method('SetRegion', 
                   'void', 
                   [param('ns3::LoraMacHelper::Regions', 'region')])
    ## lora-mac-helper.h (module 'lorawan'): static void ns3::LoraMacHelper::SetSpreadingFactorsUp(ns3::NodeContainer endDevices, ns3::NodeContainer gateways, ns3::Ptr<ns3::LoraChannel> channel) [member function]
    cls.add_method('SetSpreadingFactorsUp', 
                   'void', 
                   [param('ns3::NodeContainer', 'endDevices'), param('ns3::NodeContainer', 'gateways'), param('ns3::Ptr< ns3::LoraChannel >', 'channel')], 
                   is_static=True)
    return

def register_Ns3LoraPhyHelper_methods(root_module, cls):
    ## lora-phy-helper.h (module 'lorawan'): ns3::LoraPhyHelper::LoraPhyHelper(ns3::LoraPhyHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraPhyHelper const &', 'arg0')])
    ## lora-phy-helper.h (module 'lorawan'): ns3::LoraPhyHelper::LoraPhyHelper() [constructor]
    cls.add_constructor([])
    ## lora-phy-helper.h (module 'lorawan'): ns3::Ptr<ns3::LoraPhy> ns3::LoraPhyHelper::Create(ns3::Ptr<ns3::Node> node, ns3::Ptr<ns3::NetDevice> device) const [member function]
    cls.add_method('Create', 
                   'ns3::Ptr< ns3::LoraPhy >', 
                   [param('ns3::Ptr< ns3::Node >', 'node'), param('ns3::Ptr< ns3::NetDevice >', 'device')], 
                   is_const=True)
    ## lora-phy-helper.h (module 'lorawan'): void ns3::LoraPhyHelper::Set(std::string name, ns3::AttributeValue const & v) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'v')])
    ## lora-phy-helper.h (module 'lorawan'): void ns3::LoraPhyHelper::SetChannel(ns3::Ptr<ns3::LoraChannel> channel) [member function]
    cls.add_method('SetChannel', 
                   'void', 
                   [param('ns3::Ptr< ns3::LoraChannel >', 'channel')])
    ## lora-phy-helper.h (module 'lorawan'): void ns3::LoraPhyHelper::SetDeviceType(ns3::LoraPhyHelper::DeviceType dt) [member function]
    cls.add_method('SetDeviceType', 
                   'void', 
                   [param('ns3::LoraPhyHelper::DeviceType', 'dt')])
    return

def register_Ns3LoraTxParameters_methods(root_module, cls):
    cls.add_output_stream_operator()
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::LoraTxParameters() [constructor]
    cls.add_constructor([])
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::LoraTxParameters(ns3::LoraTxParameters const & arg0) [constructor]
    cls.add_constructor([param('ns3::LoraTxParameters const &', 'arg0')])
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::bandwidthHz [variable]
    cls.add_instance_attribute('bandwidthHz', 'double', is_const=False)
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::codingRate [variable]
    cls.add_instance_attribute('codingRate', 'uint8_t', is_const=False)
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::crcEnabled [variable]
    cls.add_instance_attribute('crcEnabled', 'bool', is_const=False)
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::headerDisabled [variable]
    cls.add_instance_attribute('headerDisabled', 'bool', is_const=False)
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::lowDataRateOptimizationEnabled [variable]
    cls.add_instance_attribute('lowDataRateOptimizationEnabled', 'bool', is_const=False)
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::nPreamble [variable]
    cls.add_instance_attribute('nPreamble', 'uint32_t', is_const=False)
    ## lora-phy.h (module 'lorawan'): ns3::LoraTxParameters::sf [variable]
    cls.add_instance_attribute('sf', 'uint8_t', is_const=False)
    return

def register_Ns3Mac48Address_methods(root_module, cls):
    cls.add_binary_comparison_operator('==')
    cls.add_binary_comparison_operator('!=')
    cls.add_binary_comparison_operator('<')
    cls.add_output_stream_operator()
    ## mac48-address.h (module 'network'): ns3::Mac48Address::Mac48Address(ns3::Mac48Address const & arg0) [constructor]
    cls.add_constructor([param('ns3::Mac48Address const &', 'arg0')])
    ## mac48-address.h (module 'network'): ns3::Mac48Address::Mac48Address() [constructor]
    cls.add_constructor([])
    ## mac48-address.h (module 'network'): ns3::Mac48Address::Mac48Address(char const * str) [constructor]
    cls.add_constructor([param('char const *', 'str')])
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::Allocate() [member function]
    cls.add_method('Allocate', 
                   'ns3::Mac48Address', 
                   [], 
                   is_static=True)
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::ConvertFrom(ns3::Address const & address) [member function]
    cls.add_method('ConvertFrom', 
                   'ns3::Mac48Address', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    ## mac48-address.h (module 'network'): void ns3::Mac48Address::CopyFrom(uint8_t const * buffer) [member function]
    cls.add_method('CopyFrom', 
                   'void', 
                   [param('uint8_t const *', 'buffer')])
    ## mac48-address.h (module 'network'): void ns3::Mac48Address::CopyTo(uint8_t * buffer) const [member function]
    cls.add_method('CopyTo', 
                   'void', 
                   [param('uint8_t *', 'buffer')], 
                   is_const=True)
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::GetBroadcast() [member function]
    cls.add_method('GetBroadcast', 
                   'ns3::Mac48Address', 
                   [], 
                   is_static=True)
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::GetMulticast(ns3::Ipv4Address address) [member function]
    cls.add_method('GetMulticast', 
                   'ns3::Mac48Address', 
                   [param('ns3::Ipv4Address', 'address')], 
                   is_static=True)
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::GetMulticast(ns3::Ipv6Address address) [member function]
    cls.add_method('GetMulticast', 
                   'ns3::Mac48Address', 
                   [param('ns3::Ipv6Address', 'address')], 
                   is_static=True)
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::GetMulticast6Prefix() [member function]
    cls.add_method('GetMulticast6Prefix', 
                   'ns3::Mac48Address', 
                   [], 
                   is_static=True)
    ## mac48-address.h (module 'network'): static ns3::Mac48Address ns3::Mac48Address::GetMulticastPrefix() [member function]
    cls.add_method('GetMulticastPrefix', 
                   'ns3::Mac48Address', 
                   [], 
                   is_static=True)
    ## mac48-address.h (module 'network'): bool ns3::Mac48Address::IsBroadcast() const [member function]
    cls.add_method('IsBroadcast', 
                   'bool', 
                   [], 
                   is_const=True)
    ## mac48-address.h (module 'network'): bool ns3::Mac48Address::IsGroup() const [member function]
    cls.add_method('IsGroup', 
                   'bool', 
                   [], 
                   is_const=True)
    ## mac48-address.h (module 'network'): static bool ns3::Mac48Address::IsMatchingType(ns3::Address const & address) [member function]
    cls.add_method('IsMatchingType', 
                   'bool', 
                   [param('ns3::Address const &', 'address')], 
                   is_static=True)
    return

def register_Ns3NetDeviceContainer_methods(root_module, cls):
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::NetDeviceContainer(ns3::NetDeviceContainer const & arg0) [constructor]
    cls.add_constructor([param('ns3::NetDeviceContainer const &', 'arg0')])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::NetDeviceContainer() [constructor]
    cls.add_constructor([])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::NetDeviceContainer(ns3::Ptr<ns3::NetDevice> dev) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::NetDevice >', 'dev')])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::NetDeviceContainer(std::string devName) [constructor]
    cls.add_constructor([param('std::string', 'devName')])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::NetDeviceContainer(ns3::NetDeviceContainer const & a, ns3::NetDeviceContainer const & b) [constructor]
    cls.add_constructor([param('ns3::NetDeviceContainer const &', 'a'), param('ns3::NetDeviceContainer const &', 'b')])
    ## net-device-container.h (module 'network'): void ns3::NetDeviceContainer::Add(ns3::NetDeviceContainer other) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::NetDeviceContainer', 'other')])
    ## net-device-container.h (module 'network'): void ns3::NetDeviceContainer::Add(ns3::Ptr<ns3::NetDevice> device) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::Ptr< ns3::NetDevice >', 'device')])
    ## net-device-container.h (module 'network'): void ns3::NetDeviceContainer::Add(std::string deviceName) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('std::string', 'deviceName')])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::Iterator ns3::NetDeviceContainer::Begin() const [member function]
    cls.add_method('Begin', 
                   'ns3::NetDeviceContainer::Iterator', 
                   [], 
                   is_const=True)
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer::Iterator ns3::NetDeviceContainer::End() const [member function]
    cls.add_method('End', 
                   'ns3::NetDeviceContainer::Iterator', 
                   [], 
                   is_const=True)
    ## net-device-container.h (module 'network'): ns3::Ptr<ns3::NetDevice> ns3::NetDeviceContainer::Get(uint32_t i) const [member function]
    cls.add_method('Get', 
                   'ns3::Ptr< ns3::NetDevice >', 
                   [param('__uint32_t', 'i')], 
                   is_const=True)
    ## net-device-container.h (module 'network'): uint32_t ns3::NetDeviceContainer::GetN() const [member function]
    cls.add_method('GetN', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    return

def register_Ns3NetworkServerHelper_methods(root_module, cls):
    ## network-server-helper.h (module 'lorawan'): ns3::NetworkServerHelper::NetworkServerHelper(ns3::NetworkServerHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::NetworkServerHelper const &', 'arg0')])
    ## network-server-helper.h (module 'lorawan'): ns3::NetworkServerHelper::NetworkServerHelper() [constructor]
    cls.add_constructor([])
    ## network-server-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::NetworkServerHelper::Install(ns3::NodeContainer c) [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::NodeContainer', 'c')])
    ## network-server-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::NetworkServerHelper::Install(ns3::Ptr<ns3::Node> node) [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::Ptr< ns3::Node >', 'node')])
    ## network-server-helper.h (module 'lorawan'): void ns3::NetworkServerHelper::SetAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## network-server-helper.h (module 'lorawan'): void ns3::NetworkServerHelper::SetEndDevices(ns3::NodeContainer endDevices) [member function]
    cls.add_method('SetEndDevices', 
                   'void', 
                   [param('ns3::NodeContainer', 'endDevices')])
    ## network-server-helper.h (module 'lorawan'): void ns3::NetworkServerHelper::SetGateways(ns3::NodeContainer gateways) [member function]
    cls.add_method('SetGateways', 
                   'void', 
                   [param('ns3::NodeContainer', 'gateways')])
    return

def register_Ns3NodeContainer_methods(root_module, cls):
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(ns3::NodeContainer const & arg0) [constructor]
    cls.add_constructor([param('ns3::NodeContainer const &', 'arg0')])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer() [constructor]
    cls.add_constructor([])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(ns3::Ptr<ns3::Node> node) [constructor]
    cls.add_constructor([param('ns3::Ptr< ns3::Node >', 'node')])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(std::string nodeName) [constructor]
    cls.add_constructor([param('std::string', 'nodeName')])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(ns3::NodeContainer const & a, ns3::NodeContainer const & b) [constructor]
    cls.add_constructor([param('ns3::NodeContainer const &', 'a'), param('ns3::NodeContainer const &', 'b')])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(ns3::NodeContainer const & a, ns3::NodeContainer const & b, ns3::NodeContainer const & c) [constructor]
    cls.add_constructor([param('ns3::NodeContainer const &', 'a'), param('ns3::NodeContainer const &', 'b'), param('ns3::NodeContainer const &', 'c')])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(ns3::NodeContainer const & a, ns3::NodeContainer const & b, ns3::NodeContainer const & c, ns3::NodeContainer const & d) [constructor]
    cls.add_constructor([param('ns3::NodeContainer const &', 'a'), param('ns3::NodeContainer const &', 'b'), param('ns3::NodeContainer const &', 'c'), param('ns3::NodeContainer const &', 'd')])
    ## node-container.h (module 'network'): ns3::NodeContainer::NodeContainer(ns3::NodeContainer const & a, ns3::NodeContainer const & b, ns3::NodeContainer const & c, ns3::NodeContainer const & d, ns3::NodeContainer const & e) [constructor]
    cls.add_constructor([param('ns3::NodeContainer const &', 'a'), param('ns3::NodeContainer const &', 'b'), param('ns3::NodeContainer const &', 'c'), param('ns3::NodeContainer const &', 'd'), param('ns3::NodeContainer const &', 'e')])
    ## node-container.h (module 'network'): void ns3::NodeContainer::Add(ns3::NodeContainer other) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::NodeContainer', 'other')])
    ## node-container.h (module 'network'): void ns3::NodeContainer::Add(ns3::Ptr<ns3::Node> node) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::Ptr< ns3::Node >', 'node')])
    ## node-container.h (module 'network'): void ns3::NodeContainer::Add(std::string nodeName) [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('std::string', 'nodeName')])
    ## node-container.h (module 'network'): ns3::NodeContainer::Iterator ns3::NodeContainer::Begin() const [member function]
    cls.add_method('Begin', 
                   'ns3::NodeContainer::Iterator', 
                   [], 
                   is_const=True)
    ## node-container.h (module 'network'): void ns3::NodeContainer::Create(uint32_t n) [member function]
    cls.add_method('Create', 
                   'void', 
                   [param('__uint32_t', 'n')])
    ## node-container.h (module 'network'): void ns3::NodeContainer::Create(uint32_t n, uint32_t systemId) [member function]
    cls.add_method('Create', 
                   'void', 
                   [param('__uint32_t', 'n'), param('__uint32_t', 'systemId')])
    ## node-container.h (module 'network'): ns3::NodeContainer::Iterator ns3::NodeContainer::End() const [member function]
    cls.add_method('End', 
                   'ns3::NodeContainer::Iterator', 
                   [], 
                   is_const=True)
    ## node-container.h (module 'network'): ns3::Ptr<ns3::Node> ns3::NodeContainer::Get(uint32_t i) const [member function]
    cls.add_method('Get', 
                   'ns3::Ptr< ns3::Node >', 
                   [param('__uint32_t', 'i')], 
                   is_const=True)
    ## node-container.h (module 'network'): static ns3::NodeContainer ns3::NodeContainer::GetGlobal() [member function]
    cls.add_method('GetGlobal', 
                   'ns3::NodeContainer', 
                   [], 
                   is_static=True)
    ## node-container.h (module 'network'): uint32_t ns3::NodeContainer::GetN() const [member function]
    cls.add_method('GetN', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    return

def register_Ns3NwkAddr_methods(root_module, cls):
    ## lora-device-address.h (module 'lorawan'): ns3::NwkAddr::NwkAddr(ns3::NwkAddr const & arg0) [constructor]
    cls.add_constructor([param('ns3::NwkAddr const &', 'arg0')])
    ## lora-device-address.h (module 'lorawan'): ns3::NwkAddr::NwkAddr(uint32_t nwkId=0) [constructor]
    cls.add_constructor([param('uint32_t', 'nwkId', default_value='0')])
    ## lora-device-address.h (module 'lorawan'): uint32_t ns3::NwkAddr::Get() const [member function]
    cls.add_method('Get', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## lora-device-address.h (module 'lorawan'): void ns3::NwkAddr::Set(uint32_t nwkAddr) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('__uint32_t', 'nwkAddr')])
    return

def register_Ns3NwkID_methods(root_module, cls):
    ## lora-device-address.h (module 'lorawan'): ns3::NwkID::NwkID(ns3::NwkID const & arg0) [constructor]
    cls.add_constructor([param('ns3::NwkID const &', 'arg0')])
    ## lora-device-address.h (module 'lorawan'): ns3::NwkID::NwkID(uint8_t nwkId=0) [constructor]
    cls.add_constructor([param('uint8_t', 'nwkId', default_value='0')])
    ## lora-device-address.h (module 'lorawan'): uint8_t ns3::NwkID::Get() const [member function]
    cls.add_method('Get', 
                   'uint8_t', 
                   [], 
                   is_const=True)
    ## lora-device-address.h (module 'lorawan'): void ns3::NwkID::Set(uint8_t nwkId) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('__uint8_t', 'nwkId')])
    return

def register_Ns3ObjectBase_methods(root_module, cls):
    ## object-base.h (module 'core'): ns3::ObjectBase::ObjectBase() [constructor]
    cls.add_constructor([])
    ## object-base.h (module 'core'): ns3::ObjectBase::ObjectBase(ns3::ObjectBase const & arg0) [constructor]
    cls.add_constructor([param('ns3::ObjectBase const &', 'arg0')])
    ## object-base.h (module 'core'): void ns3::ObjectBase::GetAttribute(std::string name, ns3::AttributeValue & value) const [member function]
    cls.add_method('GetAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue &', 'value')], 
                   is_const=True)
    ## object-base.h (module 'core'): bool ns3::ObjectBase::GetAttributeFailSafe(std::string name, ns3::AttributeValue & value) const [member function]
    cls.add_method('GetAttributeFailSafe', 
                   'bool', 
                   [param('std::string', 'name'), param('ns3::AttributeValue &', 'value')], 
                   is_const=True)
    ## object-base.h (module 'core'): ns3::TypeId ns3::ObjectBase::GetInstanceTypeId() const [member function]
    cls.add_method('GetInstanceTypeId', 
                   'ns3::TypeId', 
                   [], 
                   is_pure_virtual=True, is_const=True, is_virtual=True)
    ## object-base.h (module 'core'): static ns3::TypeId ns3::ObjectBase::GetTypeId() [member function]
    cls.add_method('GetTypeId', 
                   'ns3::TypeId', 
                   [], 
                   is_static=True)
    ## object-base.h (module 'core'): void ns3::ObjectBase::SetAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## object-base.h (module 'core'): bool ns3::ObjectBase::SetAttributeFailSafe(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetAttributeFailSafe', 
                   'bool', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## object-base.h (module 'core'): bool ns3::ObjectBase::TraceConnect(std::string name, std::string context, ns3::CallbackBase const & cb) [member function]
    cls.add_method('TraceConnect', 
                   'bool', 
                   [param('std::string', 'name'), param('std::string', 'context'), param('ns3::CallbackBase const &', 'cb')])
    ## object-base.h (module 'core'): bool ns3::ObjectBase::TraceConnectWithoutContext(std::string name, ns3::CallbackBase const & cb) [member function]
    cls.add_method('TraceConnectWithoutContext', 
                   'bool', 
                   [param('std::string', 'name'), param('ns3::CallbackBase const &', 'cb')])
    ## object-base.h (module 'core'): bool ns3::ObjectBase::TraceDisconnect(std::string name, std::string context, ns3::CallbackBase const & cb) [member function]
    cls.add_method('TraceDisconnect', 
                   'bool', 
                   [param('std::string', 'name'), param('std::string', 'context'), param('ns3::CallbackBase const &', 'cb')])
    ## object-base.h (module 'core'): bool ns3::ObjectBase::TraceDisconnectWithoutContext(std::string name, ns3::CallbackBase const & cb) [member function]
    cls.add_method('TraceDisconnectWithoutContext', 
                   'bool', 
                   [param('std::string', 'name'), param('ns3::CallbackBase const &', 'cb')])
    ## object-base.h (module 'core'): void ns3::ObjectBase::ConstructSelf(ns3::AttributeConstructionList const & attributes) [member function]
    cls.add_method('ConstructSelf', 
                   'void', 
                   [param('ns3::AttributeConstructionList const &', 'attributes')], 
                   visibility='protected')
    ## object-base.h (module 'core'): void ns3::ObjectBase::NotifyConstructionCompleted() [member function]
    cls.add_method('NotifyConstructionCompleted', 
                   'void', 
                   [], 
                   visibility='protected', is_virtual=True)
    return

def register_Ns3ObjectDeleter_methods(root_module, cls):
    ## object.h (module 'core'): ns3::ObjectDeleter::ObjectDeleter() [constructor]
    cls.add_constructor([])
    ## object.h (module 'core'): ns3::ObjectDeleter::ObjectDeleter(ns3::ObjectDeleter const & arg0) [constructor]
    cls.add_constructor([param('ns3::ObjectDeleter const &', 'arg0')])
    ## object.h (module 'core'): static void ns3::ObjectDeleter::Delete(ns3::Object * object) [member function]
    cls.add_method('Delete', 
                   'void', 
                   [param('ns3::Object *', 'object')], 
                   is_static=True)
    return

def register_Ns3ObjectFactory_methods(root_module, cls):
    cls.add_output_stream_operator()
    ## object-factory.h (module 'core'): ns3::ObjectFactory::ObjectFactory(ns3::ObjectFactory const & arg0) [constructor]
    cls.add_constructor([param('ns3::ObjectFactory const &', 'arg0')])
    ## object-factory.h (module 'core'): ns3::ObjectFactory::ObjectFactory() [constructor]
    cls.add_constructor([])
    ## object-factory.h (module 'core'): ns3::ObjectFactory::ObjectFactory(std::string typeId) [constructor]
    cls.add_constructor([param('std::string', 'typeId')])
    ## object-factory.h (module 'core'): ns3::Ptr<ns3::Object> ns3::ObjectFactory::Create() const [member function]
    cls.add_method('Create', 
                   'ns3::Ptr< ns3::Object >', 
                   [], 
                   is_const=True)
    ## object-factory.h (module 'core'): ns3::TypeId ns3::ObjectFactory::GetTypeId() const [member function]
    cls.add_method('GetTypeId', 
                   'ns3::TypeId', 
                   [], 
                   is_const=True)
    ## object-factory.h (module 'core'): void ns3::ObjectFactory::Set(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('Set', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## object-factory.h (module 'core'): void ns3::ObjectFactory::SetTypeId(ns3::TypeId tid) [member function]
    cls.add_method('SetTypeId', 
                   'void', 
                   [param('ns3::TypeId', 'tid')])
    ## object-factory.h (module 'core'): void ns3::ObjectFactory::SetTypeId(char const * tid) [member function]
    cls.add_method('SetTypeId', 
                   'void', 
                   [param('char const *', 'tid')])
    ## object-factory.h (module 'core'): void ns3::ObjectFactory::SetTypeId(std::string tid) [member function]
    cls.add_method('SetTypeId', 
                   'void', 
                   [param('std::string', 'tid')])
    return

def register_Ns3OneShotSenderHelper_methods(root_module, cls):
    ## one-shot-sender-helper.h (module 'lorawan'): ns3::OneShotSenderHelper::OneShotSenderHelper(ns3::OneShotSenderHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::OneShotSenderHelper const &', 'arg0')])
    ## one-shot-sender-helper.h (module 'lorawan'): ns3::OneShotSenderHelper::OneShotSenderHelper() [constructor]
    cls.add_constructor([])
    ## one-shot-sender-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::OneShotSenderHelper::Install(ns3::NodeContainer c) const [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::NodeContainer', 'c')], 
                   is_const=True)
    ## one-shot-sender-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::OneShotSenderHelper::Install(ns3::Ptr<ns3::Node> node) const [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::Ptr< ns3::Node >', 'node')], 
                   is_const=True)
    ## one-shot-sender-helper.h (module 'lorawan'): void ns3::OneShotSenderHelper::SetAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## one-shot-sender-helper.h (module 'lorawan'): void ns3::OneShotSenderHelper::SetSendTime(ns3::Time sendTime) [member function]
    cls.add_method('SetSendTime', 
                   'void', 
                   [param('ns3::Time', 'sendTime')])
    return

def register_Ns3PacketMetadata_methods(root_module, cls):
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::PacketMetadata(uint64_t uid, uint32_t size) [constructor]
    cls.add_constructor([param('uint64_t', 'uid'), param('uint32_t', 'size')])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::PacketMetadata(ns3::PacketMetadata const & o) [constructor]
    cls.add_constructor([param('ns3::PacketMetadata const &', 'o')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::AddAtEnd(ns3::PacketMetadata const & o) [member function]
    cls.add_method('AddAtEnd', 
                   'void', 
                   [param('ns3::PacketMetadata const &', 'o')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::AddHeader(ns3::Header const & header, uint32_t size) [member function]
    cls.add_method('AddHeader', 
                   'void', 
                   [param('ns3::Header const &', 'header'), param('__uint32_t', 'size')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::AddPaddingAtEnd(uint32_t end) [member function]
    cls.add_method('AddPaddingAtEnd', 
                   'void', 
                   [param('__uint32_t', 'end')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::AddTrailer(ns3::Trailer const & trailer, uint32_t size) [member function]
    cls.add_method('AddTrailer', 
                   'void', 
                   [param('ns3::Trailer const &', 'trailer'), param('__uint32_t', 'size')])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::ItemIterator ns3::PacketMetadata::BeginItem(ns3::Buffer buffer) const [member function]
    cls.add_method('BeginItem', 
                   'ns3::PacketMetadata::ItemIterator', 
                   [param('ns3::Buffer', 'buffer')], 
                   is_const=True)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata ns3::PacketMetadata::CreateFragment(uint32_t start, uint32_t end) const [member function]
    cls.add_method('CreateFragment', 
                   'ns3::PacketMetadata', 
                   [param('__uint32_t', 'start'), param('__uint32_t', 'end')], 
                   is_const=True)
    ## packet-metadata.h (module 'network'): uint32_t ns3::PacketMetadata::Deserialize(uint8_t const * buffer, uint32_t size) [member function]
    cls.add_method('Deserialize', 
                   'uint32_t', 
                   [param('uint8_t const *', 'buffer'), param('__uint32_t', 'size')])
    ## packet-metadata.h (module 'network'): static void ns3::PacketMetadata::Enable() [member function]
    cls.add_method('Enable', 
                   'void', 
                   [], 
                   is_static=True)
    ## packet-metadata.h (module 'network'): static void ns3::PacketMetadata::EnableChecking() [member function]
    cls.add_method('EnableChecking', 
                   'void', 
                   [], 
                   is_static=True)
    ## packet-metadata.h (module 'network'): uint32_t ns3::PacketMetadata::GetSerializedSize() const [member function]
    cls.add_method('GetSerializedSize', 
                   'uint32_t', 
                   [], 
                   is_const=True)
    ## packet-metadata.h (module 'network'): uint64_t ns3::PacketMetadata::GetUid() const [member function]
    cls.add_method('GetUid', 
                   'uint64_t', 
                   [], 
                   is_const=True)
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::RemoveAtEnd(uint32_t end) [member function]
    cls.add_method('RemoveAtEnd', 
                   'void', 
                   [param('__uint32_t', 'end')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::RemoveAtStart(uint32_t start) [member function]
    cls.add_method('RemoveAtStart', 
                   'void', 
                   [param('__uint32_t', 'start')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::RemoveHeader(ns3::Header const & header, uint32_t size) [member function]
    cls.add_method('RemoveHeader', 
                   'void', 
                   [param('ns3::Header const &', 'header'), param('__uint32_t', 'size')])
    ## packet-metadata.h (module 'network'): void ns3::PacketMetadata::RemoveTrailer(ns3::Trailer const & trailer, uint32_t size) [member function]
    cls.add_method('RemoveTrailer', 
                   'void', 
                   [param('ns3::Trailer const &', 'trailer'), param('__uint32_t', 'size')])
    ## packet-metadata.h (module 'network'): uint32_t ns3::PacketMetadata::Serialize(uint8_t * buffer, uint32_t maxSize) const [member function]
    cls.add_method('Serialize', 
                   'uint32_t', 
                   [param('uint8_t *', 'buffer'), param('__uint32_t', 'maxSize')], 
                   is_const=True)
    return

def register_Ns3PacketMetadataItem_methods(root_module, cls):
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::Item() [constructor]
    cls.add_constructor([])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::Item(ns3::PacketMetadata::Item const & arg0) [constructor]
    cls.add_constructor([param('ns3::PacketMetadata::Item const &', 'arg0')])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::current [variable]
    cls.add_instance_attribute('current', 'ns3::Buffer::Iterator', is_const=False)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::currentSize [variable]
    cls.add_instance_attribute('currentSize', 'uint32_t', is_const=False)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::currentTrimedFromEnd [variable]
    cls.add_instance_attribute('currentTrimedFromEnd', 'uint32_t', is_const=False)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::currentTrimedFromStart [variable]
    cls.add_instance_attribute('currentTrimedFromStart', 'uint32_t', is_const=False)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::isFragment [variable]
    cls.add_instance_attribute('isFragment', 'bool', is_const=False)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::tid [variable]
    cls.add_instance_attribute('tid', 'ns3::TypeId', is_const=False)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::type [variable]
    cls.add_instance_attribute('type', 'ns3::PacketMetadata::Item::ItemType', is_const=False)
    return

def register_Ns3PacketMetadataItemIterator_methods(root_module, cls):
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::ItemIterator::ItemIterator(ns3::PacketMetadata::ItemIterator const & arg0) [constructor]
    cls.add_constructor([param('ns3::PacketMetadata::ItemIterator const &', 'arg0')])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::ItemIterator::ItemIterator(ns3::PacketMetadata const * metadata, ns3::Buffer buffer) [constructor]
    cls.add_constructor([param('ns3::PacketMetadata const *', 'metadata'), param('ns3::Buffer', 'buffer')])
    ## packet-metadata.h (module 'network'): bool ns3::PacketMetadata::ItemIterator::HasNext() const [member function]
    cls.add_method('HasNext', 
                   'bool', 
                   [], 
                   is_const=True)
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item ns3::PacketMetadata::ItemIterator::Next() [member function]
    cls.add_method('Next', 
                   'ns3::PacketMetadata::Item', 
                   [])
    return

def register_Ns3PacketTagIterator_methods(root_module, cls):
    ## packet.h (module 'network'): ns3::PacketTagIterator::PacketTagIterator(ns3::PacketTagIterator const & arg0) [constructor]
    cls.add_constructor([param('ns3::PacketTagIterator const &', 'arg0')])
    ## packet.h (module 'network'): bool ns3::PacketTagIterator::HasNext() const [member function]
    cls.add_method('HasNext', 
                   'bool', 
                   [], 
                   is_const=True)
    ## packet.h (module 'network'): ns3::PacketTagIterator::Item ns3::PacketTagIterator::Next() [member function]
    cls.add_method('Next', 
                   'ns3::PacketTagIterator::Item', 
                   [])
    return

def register_Ns3PacketTagIteratorItem_methods(root_module, cls):
    ## packet.h (module 'network'): ns3::PacketTagIterator::Item::Item(ns3::PacketTagIterator::Item const & arg0) [constructor]
    cls.add_constructor([param('ns3::PacketTagIterator::Item const &', 'arg0')])
    ## packet.h (module 'network'): void ns3::PacketTagIterator::Item::GetTag(ns3::Tag & tag) const [member function]
    cls.add_method('GetTag', 
                   'void', 
                   [param('ns3::Tag &', 'tag')], 
                   is_const=True)
    ## packet.h (module 'network'): ns3::TypeId ns3::PacketTagIterator::Item::GetTypeId() const [member function]
    cls.add_method('GetTypeId', 
                   'ns3::TypeId', 
                   [], 
                   is_const=True)
    return

def register_Ns3PacketTagList_methods(root_module, cls):
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::PacketTagList() [constructor]
    cls.add_constructor([])
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::PacketTagList(ns3::PacketTagList const & o) [constructor]
    cls.add_constructor([param('ns3::PacketTagList const &', 'o')])
    ## packet-tag-list.h (module 'network'): void ns3::PacketTagList::Add(ns3::Tag const & tag) const [member function]
    cls.add_method('Add', 
                   'void', 
                   [param('ns3::Tag const &', 'tag')], 
                   is_const=True)
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData const * ns3::PacketTagList::Head() const [member function]
    cls.add_method('Head', 
                   'ns3::PacketTagList::TagData const *', 
                   [], 
                   is_const=True)
    ## packet-tag-list.h (module 'network'): bool ns3::PacketTagList::Peek(ns3::Tag & tag) const [member function]
    cls.add_method('Peek', 
                   'bool', 
                   [param('ns3::Tag &', 'tag')], 
                   is_const=True)
    ## packet-tag-list.h (module 'network'): bool ns3::PacketTagList::Remove(ns3::Tag & tag) [member function]
    cls.add_method('Remove', 
                   'bool', 
                   [param('ns3::Tag &', 'tag')])
    ## packet-tag-list.h (module 'network'): void ns3::PacketTagList::RemoveAll() [member function]
    cls.add_method('RemoveAll', 
                   'void', 
                   [])
    ## packet-tag-list.h (module 'network'): bool ns3::PacketTagList::Replace(ns3::Tag & tag) [member function]
    cls.add_method('Replace', 
                   'bool', 
                   [param('ns3::Tag &', 'tag')])
    return

def register_Ns3PacketTagListTagData_methods(root_module, cls):
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::TagData() [constructor]
    cls.add_constructor([])
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::TagData(ns3::PacketTagList::TagData const & arg0) [constructor]
    cls.add_constructor([param('ns3::PacketTagList::TagData const &', 'arg0')])
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::count [variable]
    cls.add_instance_attribute('count', 'uint32_t', is_const=False)
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::data [variable]
    cls.add_instance_attribute('data', 'uint8_t [ 1 ]', is_const=False)
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::next [variable]
    cls.add_instance_attribute('next', 'ns3::PacketTagList::TagData *', is_const=False)
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::size [variable]
    cls.add_instance_attribute('size', 'uint32_t', is_const=False)
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData::tid [variable]
    cls.add_instance_attribute('tid', 'ns3::TypeId', is_const=False)
    return

def register_Ns3PcapFile_methods(root_module, cls):
    ## pcap-file.h (module 'network'): ns3::PcapFile::PcapFile() [constructor]
    cls.add_constructor([])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Clear() [member function]
    cls.add_method('Clear', 
                   'void', 
                   [])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Close() [member function]
    cls.add_method('Close', 
                   'void', 
                   [])
    ## pcap-file.h (module 'network'): static bool ns3::PcapFile::Diff(std::string const & f1, std::string const & f2, uint32_t & sec, uint32_t & usec, uint32_t & packets, uint32_t snapLen=ns3::PcapFile::SNAPLEN_DEFAULT) [member function]
    cls.add_method('Diff', 
                   'bool', 
                   [param('std::string const &', 'f1'), param('std::string const &', 'f2'), param('uint32_t &', 'sec'), param('uint32_t &', 'usec'), param('uint32_t &', 'packets'), param('__uint32_t', 'snapLen', default_value='ns3::PcapFile::SNAPLEN_DEFAULT')], 
                   is_static=True)
    ## pcap-file.h (module 'network'): bool ns3::PcapFile::Eof() const [member function]
    cls.add_method('Eof', 
                   'bool', 
                   [], 
                   is_const=True)
    ## pcap-file.h (module 'network'): bool ns3::PcapFile::Fail() const [member function]
    cls.add_method('Fail', 
                   'bool', 
                   [], 
                   is_const=True)
    ## pcap-file.h (module 'network'): uint32_t ns3::PcapFile::GetDataLinkType() [member function]
    cls.add_method('GetDataLinkType', 
                   'uint32_t', 
                   [])
    ## pcap-file.h (module 'network'): uint32_t ns3::PcapFile::GetMagic() [member function]
    cls.add_method('GetMagic', 
                   'uint32_t', 
                   [])
    ## pcap-file.h (module 'network'): uint32_t ns3::PcapFile::GetSigFigs() [member function]
    cls.add_method('GetSigFigs', 
                   'uint32_t', 
                   [])
    ## pcap-file.h (module 'network'): uint32_t ns3::PcapFile::GetSnapLen() [member function]
    cls.add_method('GetSnapLen', 
                   'uint32_t', 
                   [])
    ## pcap-file.h (module 'network'): bool ns3::PcapFile::GetSwapMode() [member function]
    cls.add_method('GetSwapMode', 
                   'bool', 
                   [])
    ## pcap-file.h (module 'network'): int32_t ns3::PcapFile::GetTimeZoneOffset() [member function]
    cls.add_method('GetTimeZoneOffset', 
                   'int32_t', 
                   [])
    ## pcap-file.h (module 'network'): uint16_t ns3::PcapFile::GetVersionMajor() [member function]
    cls.add_method('GetVersionMajor', 
                   'uint16_t', 
                   [])
    ## pcap-file.h (module 'network'): uint16_t ns3::PcapFile::GetVersionMinor() [member function]
    cls.add_method('GetVersionMinor', 
                   'uint16_t', 
                   [])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Init(uint32_t dataLinkType, uint32_t snapLen=ns3::PcapFile::SNAPLEN_DEFAULT, int32_t timeZoneCorrection=ns3::PcapFile::ZONE_DEFAULT, bool swapMode=false, bool nanosecMode=false) [member function]
    cls.add_method('Init', 
                   'void', 
                   [param('__uint32_t', 'dataLinkType'), param('__uint32_t', 'snapLen', default_value='ns3::PcapFile::SNAPLEN_DEFAULT'), param('__int32_t', 'timeZoneCorrection', default_value='ns3::PcapFile::ZONE_DEFAULT'), param('bool', 'swapMode', default_value='false'), param('bool', 'nanosecMode', default_value='false')])
    ## pcap-file.h (module 'network'): bool ns3::PcapFile::IsNanoSecMode() [member function]
    cls.add_method('IsNanoSecMode', 
                   'bool', 
                   [])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Open(std::string const & filename, std::ios_base::openmode mode) [member function]
    cls.add_method('Open', 
                   'void', 
                   [param('std::string const &', 'filename'), param('std::_Ios_Openmode', 'mode')])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Read(uint8_t * const data, uint32_t maxBytes, uint32_t & tsSec, uint32_t & tsUsec, uint32_t & inclLen, uint32_t & origLen, uint32_t & readLen) [member function]
    cls.add_method('Read', 
                   'void', 
                   [param('uint8_t * const', 'data'), param('__uint32_t', 'maxBytes'), param('uint32_t &', 'tsSec'), param('uint32_t &', 'tsUsec'), param('uint32_t &', 'inclLen'), param('uint32_t &', 'origLen'), param('uint32_t &', 'readLen')])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Write(uint32_t tsSec, uint32_t tsUsec, uint8_t const * const data, uint32_t totalLen) [member function]
    cls.add_method('Write', 
                   'void', 
                   [param('__uint32_t', 'tsSec'), param('__uint32_t', 'tsUsec'), param('uint8_t const * const', 'data'), param('__uint32_t', 'totalLen')])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Write(uint32_t tsSec, uint32_t tsUsec, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('Write', 
                   'void', 
                   [param('__uint32_t', 'tsSec'), param('__uint32_t', 'tsUsec'), param('ns3::Ptr< ns3::Packet const >', 'p')])
    ## pcap-file.h (module 'network'): void ns3::PcapFile::Write(uint32_t tsSec, uint32_t tsUsec, ns3::Header const & header, ns3::Ptr<const ns3::Packet> p) [member function]
    cls.add_method('Write', 
                   'void', 
                   [param('__uint32_t', 'tsSec'), param('__uint32_t', 'tsUsec'), param('ns3::Header const &', 'header'), param('ns3::Ptr< ns3::Packet const >', 'p')])
    ## pcap-file.h (module 'network'): ns3::PcapFile::SNAPLEN_DEFAULT [variable]
    cls.add_static_attribute('SNAPLEN_DEFAULT', 'uint32_t const', is_const=True)
    ## pcap-file.h (module 'network'): ns3::PcapFile::ZONE_DEFAULT [variable]
    cls.add_static_attribute('ZONE_DEFAULT', 'int32_t const', is_const=True)
    return

def register_Ns3PcapHelper_methods(root_module, cls):
    ## trace-helper.h (module 'network'): ns3::PcapHelper::PcapHelper(ns3::PcapHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::PcapHelper const &', 'arg0')])
    ## trace-helper.h (module 'network'): ns3::PcapHelper::PcapHelper() [constructor]
    cls.add_constructor([])
    ## trace-helper.h (module 'network'): ns3::Ptr<ns3::PcapFileWrapper> ns3::PcapHelper::CreateFile(std::string filename, std::ios_base::openmode filemode, ns3::PcapHelper::DataLinkType dataLinkType, uint32_t snapLen=std::numeric_limits<unsigned int>::max(), int32_t tzCorrection=0) [member function]
    cls.add_method('CreateFile', 
                   'ns3::Ptr< ns3::PcapFileWrapper >', 
                   [param('std::string', 'filename'), param('std::_Ios_Openmode', 'filemode'), param('ns3::PcapHelper::DataLinkType', 'dataLinkType'), param('__uint32_t', 'snapLen', default_value='std::numeric_limits<unsigned int>::max()'), param('__int32_t', 'tzCorrection', default_value='0')])
    ## trace-helper.h (module 'network'): std::string ns3::PcapHelper::GetFilenameFromDevice(std::string prefix, ns3::Ptr<ns3::NetDevice> device, bool useObjectNames=true) [member function]
    cls.add_method('GetFilenameFromDevice', 
                   'std::string', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'device'), param('bool', 'useObjectNames', default_value='true')])
    ## trace-helper.h (module 'network'): std::string ns3::PcapHelper::GetFilenameFromInterfacePair(std::string prefix, ns3::Ptr<ns3::Object> object, uint32_t interface, bool useObjectNames=true) [member function]
    cls.add_method('GetFilenameFromInterfacePair', 
                   'std::string', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::Object >', 'object'), param('__uint32_t', 'interface'), param('bool', 'useObjectNames', default_value='true')])
    return

def register_Ns3PcapHelperForDevice_methods(root_module, cls):
    ## trace-helper.h (module 'network'): ns3::PcapHelperForDevice::PcapHelperForDevice(ns3::PcapHelperForDevice const & arg0) [constructor]
    cls.add_constructor([param('ns3::PcapHelperForDevice const &', 'arg0')])
    ## trace-helper.h (module 'network'): ns3::PcapHelperForDevice::PcapHelperForDevice() [constructor]
    cls.add_constructor([])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcap(std::string prefix, ns3::Ptr<ns3::NetDevice> nd, bool promiscuous=false, bool explicitFilename=false) [member function]
    cls.add_method('EnablePcap', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'nd'), param('bool', 'promiscuous', default_value='false'), param('bool', 'explicitFilename', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcap(std::string prefix, std::string ndName, bool promiscuous=false, bool explicitFilename=false) [member function]
    cls.add_method('EnablePcap', 
                   'void', 
                   [param('std::string', 'prefix'), param('std::string', 'ndName'), param('bool', 'promiscuous', default_value='false'), param('bool', 'explicitFilename', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcap(std::string prefix, ns3::NetDeviceContainer d, bool promiscuous=false) [member function]
    cls.add_method('EnablePcap', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::NetDeviceContainer', 'd'), param('bool', 'promiscuous', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcap(std::string prefix, ns3::NodeContainer n, bool promiscuous=false) [member function]
    cls.add_method('EnablePcap', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::NodeContainer', 'n'), param('bool', 'promiscuous', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcap(std::string prefix, uint32_t nodeid, uint32_t deviceid, bool promiscuous=false) [member function]
    cls.add_method('EnablePcap', 
                   'void', 
                   [param('std::string', 'prefix'), param('__uint32_t', 'nodeid'), param('__uint32_t', 'deviceid'), param('bool', 'promiscuous', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcapAll(std::string prefix, bool promiscuous=false) [member function]
    cls.add_method('EnablePcapAll', 
                   'void', 
                   [param('std::string', 'prefix'), param('bool', 'promiscuous', default_value='false')])
    ## trace-helper.h (module 'network'): void ns3::PcapHelperForDevice::EnablePcapInternal(std::string prefix, ns3::Ptr<ns3::NetDevice> nd, bool promiscuous, bool explicitFilename) [member function]
    cls.add_method('EnablePcapInternal', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'nd'), param('bool', 'promiscuous'), param('bool', 'explicitFilename')], 
                   is_pure_virtual=True, is_virtual=True)
    return

def register_Ns3PeriodicSenderHelper_methods(root_module, cls):
    ## periodic-sender-helper.h (module 'lorawan'): ns3::PeriodicSenderHelper::PeriodicSenderHelper(ns3::PeriodicSenderHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::PeriodicSenderHelper const &', 'arg0')])
    ## periodic-sender-helper.h (module 'lorawan'): ns3::PeriodicSenderHelper::PeriodicSenderHelper() [constructor]
    cls.add_constructor([])
    ## periodic-sender-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::PeriodicSenderHelper::Install(ns3::NodeContainer c) const [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::NodeContainer', 'c')], 
                   is_const=True)
    ## periodic-sender-helper.h (module 'lorawan'): ns3::ApplicationContainer ns3::PeriodicSenderHelper::Install(ns3::Ptr<ns3::Node> node) const [member function]
    cls.add_method('Install', 
                   'ns3::ApplicationContainer', 
                   [param('ns3::Ptr< ns3::Node >', 'node')], 
                   is_const=True)
    ## periodic-sender-helper.h (module 'lorawan'): void ns3::PeriodicSenderHelper::SetAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## periodic-sender-helper.h (module 'lorawan'): void ns3::PeriodicSenderHelper::SetPeriod(ns3::Time period) [member function]
    cls.add_method('SetPeriod', 
                   'void', 
                   [param('ns3::Time', 'period')])
    return

def register_Ns3PointToPointHelper_methods(root_module, cls):
    ## point-to-point-helper.h (module 'point-to-point'): ns3::PointToPointHelper::PointToPointHelper(ns3::PointToPointHelper const & arg0) [constructor]
    cls.add_constructor([param('ns3::PointToPointHelper const &', 'arg0')])
    ## point-to-point-helper.h (module 'point-to-point'): ns3::PointToPointHelper::PointToPointHelper() [constructor]
    cls.add_constructor([])
    ## point-to-point-helper.h (module 'point-to-point'): ns3::NetDeviceContainer ns3::PointToPointHelper::Install(ns3::NodeContainer c) [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('ns3::NodeContainer', 'c')])
    ## point-to-point-helper.h (module 'point-to-point'): ns3::NetDeviceContainer ns3::PointToPointHelper::Install(ns3::Ptr<ns3::Node> a, ns3::Ptr<ns3::Node> b) [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('ns3::Ptr< ns3::Node >', 'a'), param('ns3::Ptr< ns3::Node >', 'b')])
    ## point-to-point-helper.h (module 'point-to-point'): ns3::NetDeviceContainer ns3::PointToPointHelper::Install(ns3::Ptr<ns3::Node> a, std::string bName) [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('ns3::Ptr< ns3::Node >', 'a'), param('std::string', 'bName')])
    ## point-to-point-helper.h (module 'point-to-point'): ns3::NetDeviceContainer ns3::PointToPointHelper::Install(std::string aName, ns3::Ptr<ns3::Node> b) [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('std::string', 'aName'), param('ns3::Ptr< ns3::Node >', 'b')])
    ## point-to-point-helper.h (module 'point-to-point'): ns3::NetDeviceContainer ns3::PointToPointHelper::Install(std::string aNode, std::string bNode) [member function]
    cls.add_method('Install', 
                   'ns3::NetDeviceContainer', 
                   [param('std::string', 'aNode'), param('std::string', 'bNode')])
    ## point-to-point-helper.h (module 'point-to-point'): void ns3::PointToPointHelper::SetChannelAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetChannelAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## point-to-point-helper.h (module 'point-to-point'): void ns3::PointToPointHelper::SetDeviceAttribute(std::string name, ns3::AttributeValue const & value) [member function]
    cls.add_method('SetDeviceAttribute', 
                   'void', 
                   [param('std::string', 'name'), param('ns3::AttributeValue const &', 'value')])
    ## point-to-point-helper.h (module 'point-to-point'): void ns3::PointToPointHelper::SetQueue(std::string type, std::string n1="", ns3::AttributeValue const & v1=ns3::EmptyAttributeValue(), std::string n2="", ns3::AttributeValue const & v2=ns3::EmptyAttributeValue(), std::string n3="", ns3::AttributeValue const & v3=ns3::EmptyAttributeValue(), std::string n4="", ns3::AttributeValue const & v4=ns3::EmptyAttributeValue()) [member function]
    cls.add_method('SetQueue', 
                   'void', 
                   [param('std::string', 'type'), param('std::string', 'n1', default_value='""'), param('ns3::AttributeValue const &', 'v1', default_value='ns3::EmptyAttributeValue()'), param('std::string', 'n2', default_value='""'), param('ns3::AttributeValue const &', 'v2', default_value='ns3::EmptyAttributeValue()'), param('std::string', 'n3', default_value='""'), param('ns3::AttributeValue const &', 'v3', default_value='ns3::EmptyAttributeValue()'), param('std::string', 'n4', default_value='""'), param('ns3::AttributeValue const &', 'v4', default_value='ns3::EmptyAttributeValue()')])
    ## point-to-point-helper.h (module 'point-to-point'): void ns3::PointToPointHelper::EnableAsciiInternal(ns3::Ptr<ns3::OutputStreamWrapper> stream, std::string prefix, ns3::Ptr<ns3::NetDevice> nd, bool explicitFilename) [member function]
    cls.add_method('EnableAsciiInternal', 
                   'void', 
                   [param('ns3::Ptr< ns3::OutputStreamWrapper >', 'stream'), param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'nd'), param('bool', 'explicitFilename')], 
                   visibility='private', is_virtual=True)
    ## point-to-point-helper.h (module 'point-to-point'): void ns3::PointToPointHelper::EnablePcapInternal(std::string prefix, ns3::Ptr<ns3::NetDevice> nd, bool promiscuous, bool explicitFilename) [member function]
    cls.add_method('EnablePcapInternal', 
                   'void', 
                   [param('std::string', 'prefix'), param('ns3::Ptr< ns3::NetDevice >', 'nd'), param('bool', 'promiscuous'), param('bool', 'explicitFilename')], 
                   visibility='private', is_virtual=True)
    return

def register_Ns3SimpleRefCount__Ns3Object_Ns3ObjectBase_Ns3ObjectDeleter_methods(root_module, cls):
