/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Orange SA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "ns3/log.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/lora-net-device.h"
#include "chirpstack-helper.h"

#include "ns3/parson.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("ChirpstackHelper");

const struct coord_s ChirpstackHelper::m_center = {48.866831, 2.356719, 42};

ChirpstackHelper::ChirpstackHelper ()
{
  m_url = "http://localhost:8090/";

  /* Initialize HTTP header fields */
  str token =
      "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9."
      "eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6Ijc4ZjA4Nzc0LTZjZjUtNDI0MC04ZWMx"
      "LWRmYTYwN2I5MmYwOCIsInR5cCI6ImtleSJ9.pNYLfS8PQ7A48T1_HRrgEJcGHnDlgEPwo18D7uauKOw";
  m_header = curl_slist_append (m_header, ("Authorization: Bearer " + token).c_str ());
  m_header = curl_slist_append (m_header, "Accept: application/json");
  m_header = curl_slist_append (m_header, "Content-Type: application/json");

  /* Initialize session keys */
  m_session.netKey = "2b7e151628aed2a6abf7158809cf4f3c";
  m_session.appKey = "00000000000000000000000000000000";
}

ChirpstackHelper::~ChirpstackHelper ()
{
  CloseConnection (EXIT_SUCCESS);
}

int
ChirpstackHelper::InitConnection (Ipv4Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << (unsigned) port);

  /* Setup base URL string with IP and port */
  std::stringstream url;
  url << "http://";
  ip.Print (url);
  url << ":" << (unsigned) port;
  m_url = url.str ();
  NS_LOG_INFO ("Chirpstack REST API URL set to: " << m_url);

  /* Init curl */
  curl_global_init (CURL_GLOBAL_NOTHING);

  /* Create Ns-3 tenant */
  NewTenant ("Ns-3 Simulator");

  /* Create Ns-3 device profile */
  NewDeviceProfile ("Ns-3 Device Profile");

  /* Create Ns-3 application */
  NewApplication ("Ns-3 Application");

  return EXIT_SUCCESS;
}

void
ChirpstackHelper::CloseConnection (int signal) const
{
  str reply;

  /* Remove tentant */
  if (DELETE ("/api/tenants/" + m_session.tenantId, reply) == EXIT_FAILURE)
    NS_LOG_ERROR ("Unable to unregister tenant, got reply: " << reply);

  /* Terminate curl */
  curl_global_cleanup ();

  curl_slist_free_all (m_header); /* free the header list */

#ifdef NS3_LOG_ENABLE
  std::cout << "\nTear down process terminated after receiving signal " << signal << std::endl;
#endif // NS3_LOG_ENABLE
}

int
ChirpstackHelper::Register (Ptr<Node> node) const
{
  return RegisterPriv (node);
}

int
ChirpstackHelper::Register (NodeContainer c) const
{
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      if (RegisterPriv (*i) == EXIT_FAILURE)
        return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int
ChirpstackHelper::NewTenant (const str &name)
{
  str payload = "{"
                "  \"tenant\": {"
                "    \"canHaveGateways\": true,"
                "    \"description\": \"\","
                "    \"id\": \"\","
                "    \"maxDeviceCount\": 0,"
                "    \"maxGatewayCount\": 0,"
                "    \"name\": \"" +
                name +
                "\","
                "    \"privateGateways\": false"
                "  }"
                "}";

  str reply;
  if (POST ("/api/tenants", payload, reply) == EXIT_FAILURE)
    NS_FATAL_ERROR ("Unable to register new tenant, got reply: " << reply);

  JSON_Value *json = NULL;
  json = json_parse_string_with_comments (reply.c_str ());
  if (json == NULL)
    NS_FATAL_ERROR ("Invalid JSON in tenant registration reply: " << reply);

  m_session.tenantId = json_object_get_string (json_value_get_object (json), "id");
  json_value_free (json);

  return EXIT_SUCCESS;
}

int
ChirpstackHelper::NewDeviceProfile (const str &name)
{
  str payload = "{"
                "  \"deviceProfile\": {"
                "    \"abpRx1Delay\": 0,"
                "    \"abpRx1DrOffset\": 0,"
                "    \"abpRx2Dr\": 0,"
                "    \"abpRx2Freq\": 0,"
                "    \"adrAlgorithmId\": \"default\","
                "    \"classBPingSlotDr\": 0,"
                "    \"classBPingSlotFreq\": 0,"
                "    \"classBPingSlotPeriod\": 0,"
                "    \"classBTimeout\": 0,"
                "    \"classCTimeout\": 0,"
                "    \"description\": \"\","
                "    \"deviceStatusReqInterval\": 0,"
                "    \"flushQueueOnActivate\": false,"
                "    \"id\": \"string\","
                "    \"macVersion\": \"LORAWAN_1_0_4\","
                "    \"measurements\": {},"
                "    \"name\": \"" +
                name +
                "\","
                "    \"payloadCodecRuntime\": \"NONE\","
                "    \"payloadCodecScript\": \"string\","
                "    \"regParamsRevision\": \"RP002_1_0_3\","
                "    \"region\": \"EU868\","
                "    \"supportsClassB\": false,"
                "    \"supportsClassC\": false,"
                "    \"supportsOtaa\": false,"
                "    \"tags\": {},"
                "    \"tenantId\": \"" +
                m_session.tenantId +
                "\","
                "    \"uplinkInterval\": 600"
                "  }"
                "}";

  str reply;
  if (POST ("/api/device-profiles", payload, reply) == EXIT_FAILURE)
    NS_FATAL_ERROR ("Unable to register new device profile, got reply: " << reply);

  JSON_Value *json = NULL;
  json = json_parse_string_with_comments (reply.c_str ());
  if (json == NULL)
    NS_FATAL_ERROR ("Invalid JSON in device profile registration reply :" << reply);

  m_session.devProfId = json_object_get_string (json_value_get_object (json), "id");
  json_value_free (json);

  return EXIT_SUCCESS;
}

int
ChirpstackHelper::NewApplication (const str &name)
{
  str payload = "{"
                "  \"application\": {"
                "    \"description\": \"\","
                "    \"id\": \"\","
                "    \"name\": \"" +
                name +
                "\","
                "    \"tenantId\": \"" +
                m_session.tenantId +
                "\""
                "  }"
                "}";

  str reply;
  if (POST ("/api/applications", payload, reply) == EXIT_FAILURE)
    NS_FATAL_ERROR ("Unable to register new application, got reply: " << reply);

  JSON_Value *json = NULL;
  json = json_parse_string_with_comments (reply.c_str ());
  if (json == NULL)
    NS_FATAL_ERROR ("Invalid JSON in device profile registration reply: " << reply);

  m_session.appId = json_object_get_string (json_value_get_object (json), "id");
  json_value_free (json);

  return EXIT_SUCCESS;
}

int
ChirpstackHelper::RegisterPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<LoraNetDevice> netdev;
  // We assume nodes can have at max 1 LoraNetDevice
  for (int i = 0; i < (int) node->GetNDevices (); ++i)
    {
      netdev = node->GetDevice (i)->GetObject<LoraNetDevice> ();
      if (netdev != 0)
        {
          if (netdev->GetMac ()->GetObject<EndDeviceLorawanMac> () != 0)
            NewDevice (node);
          else if (netdev->GetMac ()->GetObject<GatewayLorawanMac> () != 0)
            NewGateway (node);
          else
            NS_FATAL_ERROR ("No LorawanMac installed (node id: " << (unsigned) node->GetId ()
                                                                 << ")");
          return EXIT_SUCCESS;
        }
    }

  NS_LOG_DEBUG ("No LoraNetDevice installed (node id: " << (unsigned) node->GetId () << ")");
  return EXIT_FAILURE;
}

int
ChirpstackHelper::NewDevice (Ptr<Node> node) const
{
  char eui[17];
  uint64_t id = node->GetId ();
  snprintf (eui, 17, "%016lx", id);

  str payload = "{"
                "  \"device\": {"
                "    \"applicationId\": \"" +
                m_session.appId +
                "\","
                "    \"description\": \"\","
                "    \"devEui\": \"" +
                str (eui) +
                "\","
                "    \"deviceProfileId\": \"" +
                m_session.devProfId +
                "\","
                "    \"isDisabled\": false,"
                "    \"name\": \"Device " +
                std::to_string ((unsigned) id) +
                "\","
                "    \"skipFcntCheck\": true,"
                "    \"tags\": {},"
                "    \"variables\": {}"
                "  }"
                "}";

  str reply;
  if (POST ("/api/devices", payload, reply) == EXIT_FAILURE)
    NS_FATAL_ERROR ("Unable to register device " << str (eui) << ", reply: " << reply);

  char devAddr[9];
  Ptr<LoraNetDevice> netdev = node->GetDevice (0)->GetObject<LoraNetDevice> ();
  Ptr<EndDeviceLorawanMac> mac = netdev->GetMac ()->GetObject<EndDeviceLorawanMac> ();
  snprintf (devAddr, 9, "%08x", mac->GetDeviceAddress ().Get ());

  payload = "{"
            "  \"deviceActivation\": {"
            "    \"aFCntDown\": 0,"
            "    \"appSKey\": \"" +
            m_session.appKey +
            "\","
            "    \"devAddr\": \"" +
            str (devAddr) +
            "\","
            "    \"fCntUp\": 0,"
            "    \"fNwkSIntKey\": \"" +
            m_session.netKey +
            "\","
            "    \"nFCntDown\": 0,"
            "    \"nwkSEncKey\": \"" +
            m_session.netKey +
            "\","
            "    \"sNwkSIntKey\": \"" +
            m_session.netKey +
            "\""
            "  }"
            "}";

  if (POST ("/api/devices/" + str (eui) + "/activate", payload, reply) == EXIT_FAILURE)
    NS_FATAL_ERROR ("Unable to activate device " << str (eui) << ", reply: " << reply);

  return EXIT_SUCCESS;
}

int
ChirpstackHelper::NewGateway (Ptr<Node> node) const
{
  char eui[17];
  uint64_t id = node->GetId ();
  snprintf (eui, 17, "%016lx", id);

  /* get reference coordinates */
  coord_s coord;
  double r_earth = 6371000.0;
  Vector position = node->GetObject<MobilityModel> ()->GetPosition ();
  coord.alt = m_center.alt + (short) position.z;
  coord.lat = m_center.lat + (position.y / r_earth) * (180.0 / M_PI);
  coord.lon =
      m_center.lon + (position.x / r_earth) * (180.0 / M_PI) / cos (m_center.lat * M_PI / 180.0);
  char coordbuf[100];
  snprintf (coordbuf, 100, "\"altitude\":%i,\"latitude\":%.5f,\"longitude\":%.5f,", coord.alt,
            coord.lat, coord.lon);

  str payload = "{"
                "  \"gateway\": {"
                "    \"description\": \"\","
                "    \"gatewayId\": \"" +
                str (eui) +
                "\","
                "    \"location\": {"
                "      \"accuracy\": 0," +
                str (coordbuf) +
                "      \"source\": \"UNKNOWN\""
                "    },"
                "    \"name\": \"Gateway " +
                std::to_string ((unsigned) id) +
                "\","
                "    \"properties\": {},"
                "    \"tags\": {},"
                "    \"tenantId\": \"" +
                m_session.tenantId +
                "\""
                "  }"
                "}";

  str reply;
  if (POST ("/api/gateways", payload, reply) == EXIT_FAILURE)
    NS_FATAL_ERROR ("Unable to register gateway " << str (eui) << ", reply: " << reply);

  return EXIT_SUCCESS;
}

int
ChirpstackHelper::POST (const str &path, const str &body, str &out) const
{
  CURL *curl;
  CURLcode res;
  std::stringstream ss;

  /* get a curl handle */
  curl = curl_easy_init ();
  if (curl)
    {
      /* Set the URL that is about to receive our POST. */
      curl_easy_setopt (curl, CURLOPT_URL, (m_url + path).c_str ());

      /* Specify the HEADER content */
      curl_easy_setopt (curl, CURLOPT_HTTPHEADER, m_header);

      /* Add body, if present */
      if (!body.empty ())
        {
          curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body.c_str ());
          curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, (long) body.size ());
        }

      /* Set reply stringstream */
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, (void *) StreamWriteCallback);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &ss);

      NS_LOG_INFO ("Sending POST request to " << m_url << path << ", with body: " << body);
      /* Perform the request, res will get the return code */
      res = curl_easy_perform (curl);

      /* always cleanup */
      curl_easy_cleanup (curl);
    }

  out = ss.str ();
  NS_LOG_INFO ("Received POST reply: " << out);

  /* Check for errors */
  if (res != CURLE_OK)
    {
      NS_LOG_ERROR ("curl_easy_perform() failed: " << curl_easy_strerror (res) << "\n");
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int
ChirpstackHelper::DELETE (const str &path, str &out) const
{
  CURL *curl;
  CURLcode res;
  std::stringstream ss;

  /* get a curl handle */
  curl = curl_easy_init ();
  if (curl)
    {
      /* Set the URL that is about to receive our POST. */
      curl_easy_setopt (curl, CURLOPT_URL, (m_url + path).c_str ());

      /* Specify the HEADER content */
      curl_easy_setopt (curl, CURLOPT_HTTPHEADER, m_header);

      /* DELETE the given path */
      curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "DELETE");

      /* Set reply stringstream */
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, (void *) StreamWriteCallback);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &ss);

      NS_LOG_INFO ("Sending DELETE request to " << m_url << path);
      /* Perform the request, res will get the return code */
      res = curl_easy_perform (curl);

      /* always cleanup */
      curl_easy_cleanup (curl);
    }

  out = ss.str ();
  NS_LOG_INFO ("Received DELETE reply: " << out);

  /* Check for errors */
  if (res != CURLE_OK)
    {
      NS_LOG_ERROR ("curl_easy_perform() failed: " << curl_easy_strerror (res) << "\n");
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

size_t
ChirpstackHelper::StreamWriteCallback (char *buffer, size_t size, size_t nitems,
                                       std::ostream *stream)
{
  size_t realwrote = size * nitems;
  stream->write (buffer, static_cast<std::streamsize> (realwrote));
  if (!(*stream))
    realwrote = 0;

  return realwrote;
}

} // namespace lorawan
} // namespace ns3
