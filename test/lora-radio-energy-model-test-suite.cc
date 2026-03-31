// Include headers of classes to test
#include "ns3/basic-energy-source-helper.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/file-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("LoraRadioEnergyModelTestSuite");

/**
 * @ingroup lorawan
 *
 * It tests the correct behavior when the energy of the LoraRadioEnergyModel is depleted
 */
class EnergyDepletionTest : public TestCase
{
  public:
    EnergyDepletionTest();
    ~EnergyDepletionTest() override;

  private:
    /// Depletion handler function that counts the depletion
    static void DepletionHandler(Ptr<EndDeviceLoraPhy> loraPhy);
    void DoRun() override;

    static int s_depletionCount; ///< depletion count
};

EnergyDepletionTest::EnergyDepletionTest()
    : TestCase("Verify that the LoraRadioEnergyModel correctly depletes the energy")
{
}

EnergyDepletionTest::~EnergyDepletionTest()
{
}

int EnergyDepletionTest::s_depletionCount = 0;

void
EnergyDepletionTest::DepletionHandler(Ptr<EndDeviceLoraPhy> loraPhy)
{
    s_depletionCount++;
    loraPhy->SwitchToOff();
}

void
EnergyDepletionTest::DoRun()
{
    s_depletionCount = 0;
    /************************
     *  Create the channel  *
     ************************/

    // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    /************************
     *  Create the helpers  *
     ************************/

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(100, 0, 0));
    allocator->Add(Vector(0, 0, 0));
    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();

    // Create the LoraHelper
    LoraHelper helper = LoraHelper();

    /************************
     *  Create End Devices  *
     ************************/
    // Create a set of nodes
    NodeContainer endDevices;
    endDevices.Create(1);

    // Assign a mobility model to the node
    mobility.Install(endDevices);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    NetDeviceContainer endDevicesNetDevices = helper.Install(phyHelper, macHelper, endDevices);

    /*********************
     *  Create Gateways  *
     *********************/

    NodeContainer gateways;
    gateways.Create(1);

    mobility.SetPositionAllocator(allocator);
    mobility.Install(gateways);

    // Create a netdevice for each gateway
    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    /*********************************************
     *  Install applications on the end devices  *
     *********************************************/

    PeriodicSenderHelper periodicSenderHelper;
    periodicSenderHelper.SetPeriod(Seconds(5));

    periodicSenderHelper.Install(endDevices);

    /************************
     * Install Energy Model *
     ************************/

    BasicEnergySourceHelper basicSourceHelper;
    LoraRadioEnergyModelHelper radioEnergyHelper;

    // configure energy source
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(1)); // Energy in J
    basicSourceHelper.Set("BasicEnergySupplyVoltageV", DoubleValue(3.3));
    basicSourceHelper.Set("BasicEnergyLowBatteryThreshold", DoubleValue(0.1)); // Not Completely
    // drained, the default value is 0.1

    radioEnergyHelper.Set("StandbyCurrentA", DoubleValue(0.0014));
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.028));
    radioEnergyHelper.Set("SleepCurrentA", DoubleValue(0.0000015));
    radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.0112));

    radioEnergyHelper.SetTxCurrentModel("ns3::ConstantLoraTxCurrentModel",
                                        "TxCurrent",
                                        DoubleValue(0.028));

    // install source on end devices' nodes
    EnergySourceContainer sources = basicSourceHelper.Install(endDevices);

    // install device model
    DeviceEnergyModelContainer deviceEnergyModels =
        radioEnergyHelper.Install(endDevicesNetDevices, sources);

    // set the depletion callback
    auto loraNetDevice = DynamicCast<LoraNetDevice>(endDevicesNetDevices.Get(0));
    auto loraPhy = DynamicCast<EndDeviceLoraPhy>(loraNetDevice->GetPhy());
    deviceEnergyModels.Get(0)->GetObject<LoraRadioEnergyModel>()->SetEnergyDepletionCallback(
        MakeBoundCallback(DepletionHandler, loraPhy));

    /****************
     *  Simulation  *
     ****************/

    Simulator::Stop(Seconds(1000));

    Simulator::Run();

    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(s_depletionCount, 1, "Depletion callback not invoked");
}

// --------------------------------------------------------------------------- //
/**
 * @ingroup lorawan
 * @ingroup tests
 *
 * @brief Test Suite for the LoraRadioEnergyModel
 */
class LoraRadioEnergyModelTestSuite : public TestSuite
{
  public:
    LoraRadioEnergyModelTestSuite(); //!< Default constructor
};

LoraRadioEnergyModelTestSuite::LoraRadioEnergyModelTestSuite()
    : TestSuite("lora-radio-energy-model", Type::SYSTEM)
{
    AddTestCase(new EnergyDepletionTest(), TestCase::Duration::QUICK);
}

// create an instance of the test suite
static LoraRadioEnergyModelTestSuite g_loraRadioEnergyModelTestSuite;
