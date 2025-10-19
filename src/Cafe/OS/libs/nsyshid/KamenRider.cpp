#include "KamenRider.h"

#include <random>

#include "nsyshid.h"
#include "Backend.h"

#include "Common/FileStream.h"

namespace nsyshid
{
	RideGateUSB g_kamenridegate;

	const std::map<const std::pair<const uint8, const uint8>, const char*> s_listKamenRiders = {
		{{0x10, 0x10}, "Kamen Rider Drive Wind"},
		{{0x10, 0x20}, "Kamen Rider Drive Water"},
		{{0x10, 0x30}, "Kamen Rider Drive Fire"},
		{{0x10, 0x40}, "Kamen Rider Drive Light"},
		{{0x10, 0x50}, "Kamen Rider Drive Dark"},
		{{0x11, 0x10}, "Kamen Rider Gaim Wind"},
		{{0x11, 0x20}, "Kamen Rider Gaim Water"},
		{{0x12, 0x20}, "Kamen Rider Wizard Water"},
		{{0x12, 0x30}, "Kamen Rider Wizard Fire"},
		{{0x13, 0x40}, "Kamen Rider Fourze Light"},
		{{0x14, 0x20}, "Kamen Rider 000 Water"},
		{{0x15, 0x10}, "Kamen Rider Double Wind"},
		{{0x16, 0x50}, "Kamen Rider Decade Dark"},
		{{0x17, 0x50}, "Kamen Rider Kiva Dark"},
		{{0x18, 0x40}, "Kamen Rider Den-O Light"},
		{{0x19, 0x30}, "Kamen Rider Kabuto Fire"},
		{{0x1A, 0x30}, "Kamen Rider Hibiki Fire"},
		{{0x1B, 0x50}, "Kamen Rider Blade Dark"},
		{{0x1C, 0x50}, "Kamen Rider Faiz Dark"},
		{{0x1D, 0x10}, "Kamen Rider Ryuki Wind"},
		{{0x1E, 0x20}, "Kamen Rider Agito Water"},
		{{0x1F, 0x40}, "Kamen Rider Kuuga Light"},
	};

	const std::map<const uint8, const char*> s_listChips = {
		{0x20, "Type Wild"},
		{0x21, "Kamen Rider Zangetsu"},
		{0x22, "All Dragon"},
		{0x31, "Kachidoki Arms"},
	};

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
		cemuLog_log(LogType::Force, "Ride gate read: \n{}", HexDump(message->data, message->length));
		return Device::ReadResult::Success;
	}

	Device::WriteResult KamenRiderGateDevice::Write(WriteMessage* message)
	{
		cemuLog_log(LogType::Force, "Ride gate write: \n{}", HexDump(message->data, message->length));
		const uint8 length = message->length;
		g_kamenridegate.SendCommand(std::span<const uint8>{message->data, length}, length);
		message->bytesWritten = message->length;
		return Device::WriteResult::Success;
	}

	bool KamenRiderGateDevice::GetDescriptor(uint8 descType,
											 uint8 descIndex,
											 uint16 lang,
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

	bool KamenRiderGateDevice::SetIdle(uint8 ifIndex,
									   uint8 reportId,
									   uint8 duration)
	{
		return true;
	}

	bool KamenRiderGateDevice::SetProtocol(uint8 ifIndex, uint8 protocol)
	{
		cemuLog_log(LogType::Force, "Ride gate protocol");
		return true;
	}

	bool KamenRiderGateDevice::SetReport(ReportMessage* message)
	{
		cemuLog_log(LogType::Force, "Ride gate report: \n{}", HexDump(message->data, message->length));
		return true;
	}

	std::array<uint8, 64> RideGateUSB::GetStatus()
	{
		std::array<uint8, 64> response = {};

		bool responded = false;
		do
		{
			if (!m_figureAddedRemovedResponses.empty() && m_isAwake)
			{
				std::lock_guard lock(m_kamenRiderMutex);
				response = m_figureAddedRemovedResponses.front();
				m_figureAddedRemovedResponses.pop();
				responded = true;
			}
			else if (!m_queries.empty())
			{
				response = m_queries.front();
				m_queries.pop();
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

	void RideGateUSB::SendCommand(std::span<const uint8> buf, const uint8 length)
	{
		const uint8 command = buf[2];
		const uint8 sequence = buf[3];

		std::array<uint8, 64> q_result{};

		switch (command)
		{
		case 0xB0: // Wake
		{
			m_isAwake = true;
			q_result = {0x55, 0x1a, command, sequence, 0x00, 0x07, 0x00, 0x03, 0x02,
						0x09, 0x20, 0x03, 0xf5, 0x00, 0x19, 0x42, 0x52, 0xb7,
						0xb9, 0xa1, 0xae, 0x2b, 0x88, 0x42, 0x05, 0xfe, 0xe0, 0x1c, 0xac};
			break;
		}
		case 0xC0:
		case 0xC3: // Color Commands
		{
			g_kamenridegate.GetBlankResponse(command, sequence, q_result);
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
			g_kamenridegate.QueryBlock(q_result, command, sequence, &buf[4], buf[11], buf[12]);
			break;
		}
		case 0xD3:
		{
			// Write 16 bytes to figure with UID buf[4] - buf[10]
			g_kamenridegate.WriteBlock(q_result, command, sequence, &buf[4], buf[11], buf[12], &buf[13]);
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

	void RideGateUSB::QueryBlock(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence, const uint8* uid, uint8 sector, uint8 block)
	{
		replyBuf = {0x55, 0x13, command, sequence, 0x00};

		std::array<uint8, 7> uid_array = {uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]};

		RiderFigure& figure = GetFigureByUID(uid_array);
		if (figure.present)
		{
			if (sector < 5 && block < 4)
			{
				memcpy(&replyBuf[5], &figure.data[(sector * 4 * 16) + (block * 16)], 16);
			}
		}
		replyBuf[21] = GenerateChecksum(replyBuf, 21);
	}

	void RideGateUSB::WriteBlock(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence, const uint8* uid, uint8 sector, uint8 block, const uint8* toWriteBuf)
	{
		std::array<uint8, 7> uid_array = {uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]};

		RiderFigure& figure = GetFigureByUID(uid_array);
		if (figure.present)
		{
			if (sector < 5 && block < 4)
			{
				memcpy(&figure.data[(sector * 4 * 16) + (block * 16)], toWriteBuf, 16);
			}
		}

		GetBlankResponse(command, sequence, replyBuf);
	}

	void RideGateUSB::GetBlankResponse(uint8 command, uint8 sequence, std::array<uint8, 64>& replyBuf)
	{
		replyBuf = {0x55, 0x02, command, sequence};
		replyBuf[4] = GenerateChecksum(replyBuf, 4);
	}

	static uint32 SummonrideCRC32(const std::array<uint8, 16>& buffer)
	{
		static constexpr std::array<uint32, 256> CRC32_TABLE{
			0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535,
			0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd,
			0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d,
			0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
			0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
			0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
			0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac,
			0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
			0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
			0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
			0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb,
			0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
			0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea,
			0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce,
			0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
			0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
			0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409,
			0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
			0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739,
			0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
			0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344, 0x8708a3d2, 0x1e01f268,
			0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0,
			0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8,
			0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
			0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
			0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703,
			0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
			0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
			0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae,
			0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
			0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 0x88085ae6,
			0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
			0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d,
			0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5,
			0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
			0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
			0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

		// Kamen Rider figures calculate their CRC32 based on 12 bytes in the block of 16
		uint32 ret = 0;
		for (uint32 i = 0; i < 12; ++i)
		{
			uint8 index = uint8(ret & 0xFF) ^ buffer[i];
			ret = ((ret >> 8) ^ CRC32_TABLE[index]);
		}

		return ret;
	}

	uint8 RideGateUSB::LoadFigure(const std::array<uint8, 0x14 * 0x10>& buf, std::unique_ptr<FileStream> file)
	{
		std::lock_guard lock(m_kamenRiderMutex);

		uint8 foundSlot = 0xFF;

		// mimics spot retaining on the portal
		for (auto i = 0; i < 7; i++)
		{
			if (!m_figures[i].present)
			{
				if (i < foundSlot)
				{
					foundSlot = i;
				}
			}
		}

		if (foundSlot != 0xFF)
		{
			auto& figure = m_figures[foundSlot];
			memcpy(figure.data.data(), buf.data(), buf.size());
			figure.kamenFile = std::move(file);
			figure.uid = {buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]};
			figure.present = true;

			if (m_isAwake)
			{
				std::array<uint8, 64> figureAddedResponse = {0x56, 0x09, 0x09, 0x01};
				memcpy(&figureAddedResponse[4], figure.uid.data(), figure.uid.size());
				figureAddedResponse[11] = GenerateChecksum(figureAddedResponse, 11);
				m_figureAddedRemovedResponses.push(figureAddedResponse);
			}
		}
		return foundSlot;
	}

	bool RideGateUSB::RemoveFigure(uint8 index)
	{
		std::lock_guard lock(m_kamenRiderMutex);

		auto& figure = m_figures[index];

		if (figure.present)
		{
			figure.present = false;
			figure.Save();
			figure.kamenFile.reset();
			if (m_isAwake)
			{
				std::array<uint8, 64> figureRemovedResponse = {0x56, 0x09, 0x09, 0x00};
				memcpy(&figureRemovedResponse[4], figure.uid.data(), figure.uid.size());
				figureRemovedResponse[11] = GenerateChecksum(figureRemovedResponse, 11);
				m_figureAddedRemovedResponses.push(figureRemovedResponse);
			}
			figure.uid = {};
			return true;
		}

		return false;
	}

	bool RideGateUSB::CreateFigure(fs::path pathName, uint8 type, uint8 id)
	{
		FileStream* riderFile(FileStream::createFile2(pathName));
		if (!riderFile)
			return false;

		std::array<uint8, 0x14 * 0x10> fileData{};
		fileData[0] = 0x04;
		fileData[6] = 0x80;

		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dist(0, 255);

		fileData[1] = dist(mt);
		fileData[2] = dist(mt);
		fileData[3] = dist(mt);
		fileData[4] = dist(mt);
		fileData[5] = dist(mt);

		fileData[7] = 0x89;
		fileData[8] = 0x44;
		fileData[10] = 0xc2;

		std::array<uint8, 16> figureData = {uint8(dist(mt)), 0x03, 0x00, 0x00, 0x01, 0x0e, 0x0a, 0x0a, 0x10, type, 0x01, id};
		uint32 checksum = SummonrideCRC32(figureData);
		for (sint8 i = 3; i >= 0; i--)
		{
			figureData[15 - i] = uint8((checksum >> (3 - i) * 8) & 0xFF);
		}
		memcpy(&fileData[16], figureData.data(), figureData.size());

		if (fileData.size() != riderFile->writeData(fileData.data(), fileData.size()))
		{
			delete riderFile;
			return false;
		}
		delete riderFile;
		return true;
	}

	std::string RideGateUSB::FindFigure(uint8 type, uint8 id)
	{
		if (type == 0x00)
		{
			for (const auto& it : GetChipList())
			{
				if (it.first == id)
				{
					return it.second;
				}
			}
			return std::format("Unknown Chip ({})", id);
		}
		for (const auto& it : GetRiderList())
		{
			if (it.first.first == id && it.first.second == type)
			{
				return it.second;
			}
		}
		switch (type)
		{
		case 0x10:
			return std::format("Unknown Rider Wind ({})", id);
		case 0x20:
			return std::format("Unknown Rider Water ({})", id);
		case 0x30:
			return std::format("Unknown Rider Fire ({})", id);
		case 0x40:
			return std::format("Unknown Rider Light ({})", id);
		case 0x50:
			return std::format("Unknown Rider Dark ({})", id);
		default:
			return std::format("Unknown Rider ({})", id);
		}
	}

	std::map<const std::pair<const uint8, const uint8>, const char*> RideGateUSB::GetRiderList()
	{
		return s_listKamenRiders;
	}

	std::map<const uint8, const char*> RideGateUSB::GetChipList()
	{
		return s_listChips;
	}

	RideGateUSB::RiderFigure& RideGateUSB::GetFigureByUID(const std::array<uint8, 7> uid)
	{
		for (uint8 i = 0; i < m_figures.size(); i++)
		{
			if (m_figures[i].uid == uid)
			{
				return m_figures[i];
			}
		}
		return m_figures[7];
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

	void RideGateUSB::RiderFigure::Save()
	{
		if (!kamenFile)
			return;

		kamenFile->SetPosition(0);
		kamenFile->writeData(data.data(), data.size());
	}
} // namespace nsyshid