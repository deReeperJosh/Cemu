#include "KamenRider.h"

#include <random>

#include "nsyshid.h"
#include "Backend.h"

#include "Common/FileStream.h"

namespace nsyshid
{
	RideGateUSB g_kamenridegate;

	KamenRiderGateDevice::KamenRiderGateDevice()
		: Device(0x0E6F, 0x200A, 1, 2, 0)
	{
		m_IsOpened = false;
	}

	bool KamenRiderGateDevice::Open()
	{
		if (!IsOpened())
		{
			m_IsOpened = true;
		}
		return true;
	}

	void KamenRiderGateDevice::Close()
	{
		if (IsOpened())
		{
			m_IsOpened = false;
		}
	}

	bool KamenRiderGateDevice::IsOpened()
	{
		return m_IsOpened;
	}

	std::string HexDump(const uint8* data, size_t size)
	{
		constexpr size_t BYTES_PER_LINE = 16;

		std::string out;
		for (size_t row_start = 0; row_start < size; row_start += BYTES_PER_LINE)
		{
			out += fmt::format("{:06x}: ", row_start);
			for (size_t i = 0; i < BYTES_PER_LINE; ++i)
			{
				if (row_start + i < size)
				{
					out += fmt::format("{:02x} ", data[row_start + i]);
				}
				else
				{
					out += "   ";
				}
			}
			out += " ";
			for (size_t i = 0; i < BYTES_PER_LINE; ++i)
			{
				if (row_start + i < size)
				{
					char c = static_cast<char>(data[row_start + i]);
					out += std::isprint(c, std::locale::classic()) ? c : '.';
				}
			}
			out += "\n";
		}
		return out;
	}

	Device::ReadResult KamenRiderGateDevice::Read(ReadMessage* message)
	{
		memcpy(message->data, g_kamenridegate.GetStatus().data(), message->length);
		message->bytesRead = message->length;
		return Device::ReadResult::Success;
	}

	Device::WriteResult KamenRiderGateDevice::Write(WriteMessage* message)
	{
		cemuLog_log(LogType::Force, "Ride gate write: \n{}", HexDump(message->data, message->length));
		const uint8 length = message->length;
		g_kamenridegate.SendCommand(std::span<const uint8>{message->data, length});
		message->bytesWritten = message->length;
		return Device::WriteResult::Success;
	}

	bool KamenRiderGateDevice::GetDescriptor(uint8 descType,
											 uint8 descIndex,
											 uint8 lang,
											 uint8* output,
											 uint32 outputMaxLength)
	{
		uint8 configurationDescriptor[0x29];

		uint8* currentWritePtr;

		// configuration descriptor
		currentWritePtr = configurationDescriptor + 0;
		*(uint8*)(currentWritePtr + 0) = 9;			// bLength
		*(uint8*)(currentWritePtr + 1) = 2;			// bDescriptorType
		*(uint16be*)(currentWritePtr + 2) = 0x0029; // wTotalLength
		*(uint8*)(currentWritePtr + 4) = 1;			// bNumInterfaces
		*(uint8*)(currentWritePtr + 5) = 1;			// bConfigurationValue
		*(uint8*)(currentWritePtr + 6) = 0;			// iConfiguration
		*(uint8*)(currentWritePtr + 7) = 0x80;		// bmAttributes
		*(uint8*)(currentWritePtr + 8) = 0xFA;		// MaxPower
		currentWritePtr = currentWritePtr + 9;
		// configuration descriptor
		*(uint8*)(currentWritePtr + 0) = 9;	   // bLength
		*(uint8*)(currentWritePtr + 1) = 0x04; // bDescriptorType
		*(uint8*)(currentWritePtr + 2) = 0;	   // bInterfaceNumber
		*(uint8*)(currentWritePtr + 3) = 0;	   // bAlternateSetting
		*(uint8*)(currentWritePtr + 4) = 2;	   // bNumEndpoints
		*(uint8*)(currentWritePtr + 5) = 3;	   // bInterfaceClass
		*(uint8*)(currentWritePtr + 6) = 0;	   // bInterfaceSubClass
		*(uint8*)(currentWritePtr + 7) = 0;	   // bInterfaceProtocol
		*(uint8*)(currentWritePtr + 8) = 0;	   // iInterface
		currentWritePtr = currentWritePtr + 9;
		// configuration descriptor
		*(uint8*)(currentWritePtr + 0) = 9;			// bLength
		*(uint8*)(currentWritePtr + 1) = 0x21;		// bDescriptorType
		*(uint16be*)(currentWritePtr + 2) = 0x0111; // bcdHID
		*(uint8*)(currentWritePtr + 4) = 0x00;		// bCountryCode
		*(uint8*)(currentWritePtr + 5) = 0x01;		// bNumDescriptors
		*(uint8*)(currentWritePtr + 6) = 0x22;		// bDescriptorType
		*(uint16be*)(currentWritePtr + 7) = 0x001D; // wDescriptorLength
		currentWritePtr = currentWritePtr + 9;
		// endpoint descriptor 1
		*(uint8*)(currentWritePtr + 0) = 7;		  // bLength
		*(uint8*)(currentWritePtr + 1) = 0x05;	  // bDescriptorType
		*(uint8*)(currentWritePtr + 2) = 0x81;	  // bEndpointAddress
		*(uint8*)(currentWritePtr + 3) = 0x03;	  // bmAttributes
		*(uint16be*)(currentWritePtr + 4) = 0x40; // wMaxPacketSize
		*(uint8*)(currentWritePtr + 6) = 0x01;	  // bInterval
		currentWritePtr = currentWritePtr + 7;
		// endpoint descriptor 2
		*(uint8*)(currentWritePtr + 0) = 7;		  // bLength
		*(uint8*)(currentWritePtr + 1) = 0x05;	  // bDescriptorType
		*(uint8*)(currentWritePtr + 2) = 0x02;	  // bEndpointAddress
		*(uint8*)(currentWritePtr + 3) = 0x03;	  // bmAttributes
		*(uint16be*)(currentWritePtr + 4) = 0x40; // wMaxPacketSize
		*(uint8*)(currentWritePtr + 6) = 0x01;	  // bInterval
		currentWritePtr = currentWritePtr + 7;

		cemu_assert_debug((currentWritePtr - configurationDescriptor) == 0x29);

		memcpy(output, configurationDescriptor,
			   std::min<uint32>(outputMaxLength, sizeof(configurationDescriptor)));
		return true;
	}

	bool KamenRiderGateDevice::SetProtocol(uint8 ifIndex, uint8 protocol)
	{
		cemuLog_log(LogType::Force, "Ride gate protocol");
		return true;
	}

	bool KamenRiderGateDevice::SetReport(ReportMessage* message)
	{
		cemuLog_log(LogType::Force, "Ride gate write: \n{}", HexDump(message->originalData, message->originalLength));
		return true;
	}

	std::array<uint8, 64> RideGateUSB::GetStatus()
	{
		std::array<uint8, 64> response = {};

		bool responded = false;
		do
		{
			if (!m_queries.empty())
			{
				response = m_queries.front();
				m_queries.pop();
				responded = true;
			}
			else if (!m_figureAddedRemovedResponses.empty())
			{
				std::lock_guard lock(m_kamenRiderMutex);
				response = m_figureAddedRemovedResponses.front();
				m_figureAddedRemovedResponses.pop();
				responded = true;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
		while (responded == false);
		return response;
	}

	void RideGateUSB::SendCommand(std::span<const uint8> buf)
	{
		const uint8 command = buf[2];
		const uint8 sequence = buf[3];

		std::array<uint8, 64> q_result{};

		switch (command)
		{
		case 0xB0: // Wake
		{
			q_result = {0x55, 0x1a, 0xb0, sequence, 0x00, 0x07, 0x00, 0x03, 0x02,
						0x09, 0x20, 0x03, 0xf5, 0x00, 0x19, 0x42, 0x52, 0xb7,
						0xb9, 0xa1, 0xae, 0x2b, 0x88, 0x42, 0x05, 0xfe, 0xe0, 0x1c, 0xac};
			break;
		}
		case 0xC0:
		case 0xC3: // Color Commands
		{
			q_result = {0x55, 0x02, command, sequence};
			q_result[4] = GenerateChecksum(q_result, 4);
			break;
		}
		case 0xD0: // Tag List
		{
			// Return list of figure UIDs, separated by an 09
			g_kamenridegate.GetListTags(q_result, command, sequence);
			break;
		}
		case 0xD2: // Read
		{
			// Read 16 bytes from figure with UID buf[4] - buf[10]
			break;
		}
		default:
			cemuLog_log(LogType::Force, "Unknown Kamen Rider Function: {:x}", command);
			break;
		}

		m_queries.push(q_result);
	}

	void RideGateUSB::GetListTags(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence)
	{
		replyBuf = {0x55, 0x02, command, sequence};
		uint8 index = 4;
		for (auto& figure : m_figures)
		{
			if (figure.present)
			{
				replyBuf[index] = 0x09;
				memcpy(&replyBuf[index + 1], figure.data.data(), 7);
				index += 8;
				replyBuf[1] += 8;
			}
		}
		replyBuf[index] = GenerateChecksum(replyBuf, index);
	}

	void RideGateUSB::QueryBlock(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence, const std::array<uint8, 7> uid, uint8 block, uint8 line)
	{
		replyBuf = {0x55, 0x13, command, sequence, 0x00};

		auto figure = GetFigureByUID(uid);
		if (figure)
		{
			
		}
		replyBuf[21] = GenerateChecksum(replyBuf, 21);
	}

	std::optional<RideGateUSB::RiderFigure&> RideGateUSB::GetFigureByUID(const std::array<uint8, 7> uid)
	{
		for (auto& figure : m_figures)
		{
			if (figure.uid == uid)
			{
				return figure;
			}
		}
		return std::nullopt;
	}

	uint8 RideGateUSB::GenerateChecksum(const std::array<uint8, 64>& data,
										int numOfBytes) const
	{
		int checksum = 0;
		for (int i = 0; i < numOfBytes; i++)
		{
			checksum += data[i];
		}
		return (checksum & 0xFF);
	}
} // namespace nsyshid