#include "Dimensions.h"

#include "nsyshid.h"
#include "Backend.h"

#include "Common/FileStream.h"

#include <random>

namespace nsyshid
{
	static constexpr std::array<uint8, 16> KEY = {
		0x55, 0xFE, 0xF6, 0xB0, 0x62, 0xBF, 0x0B, 0x41,
		0xC9, 0xB3, 0x7C, 0xB4, 0x97, 0x3E, 0x29, 0x7B};

	DimensionsUSB g_dimensionstoypad;

	const std::map<const uint16, const std::string> list_minis = {
		{1, "Batman"},
		{2, "Gandalf"},
		{3, "Wyldstyle"},
		{4, "Aquaman"},
		{5, "Bad Cop"},
		{6, "Bane"},
		{7, "Bart Simpson"},
		{8, "Benny"},
		{9, "Chell"},
		{10, "Cole"},
		{11, "Cragger"},
		{12, "Cyborg"},
		{13, "Cyberman"},
		{14, "Doc Brown"},
		{15, "The Doctor"},
		{16, "Emmet"},
		{17, "Eris"},
		{18, "Gimli"},
		{19, "Gollum"},
		{20, "Harley Quinn"},
		{21, "Homer Simpson"},
		{22, "Jay"},
		{23, "Joker"},
		{24, "Kai"},
		{25, "ACU Trooper"},
		{26, "Gamer Kid"},
		{27, "Krusty the Clown"},
		{28, "Laval"},
		{29, "Legolas"},
		{30, "Lloyd"},
		{31, "Marty McFly"},
		{32, "Nya"},
		{33, "Owen Grady"},
		{34, "Peter Venkman"},
		{35, "Slimer"},
		{36, "Scooby-Doo"},
		{37, "Sensei Wu"},
		{38, "Shaggy"},
		{39, "Stay Puft"},
		{40, "Superman"},
		{41, "Unikitty"},
		{42, "Wicked Witch of the West"},
		{43, "Wonder Woman"},
		{44, "Zane"},
		{45, "Green Arrow"},
		{46, "Supergirl"},
		{47, "Abby Yates"},
		{48, "Finn the Human"},
		{49, "Ethan Hunt"},
		{50, "Lumpy Space Princess"},
		{51, "Jake the Dog"},
		{52, "Harry Potter"},
		{53, "Lord Voldemort"},
		{54, "Michael Knight"},
		{55, "B.A. Baracus"},
		{56, "Newt Scamander"},
		{57, "Sonic the Hedgehog"},
		{58, "Future Update (unreleased)"},
		{59, "Gizmo"},
		{60, "Stripe"},
		{61, "E.T."},
		{62, "Tina Goldstein"},
		{63, "Marceline the Vampire Queen"},
		{64, "Batgirl"},
		{65, "Robin"},
		{66, "Sloth"},
		{67, "Hermione Granger"},
		{68, "Chase McCain"},
		{69, "Excalibur Batman"},
		{70, "Raven"},
		{71, "Beast Boy"},
		{72, "Betelgeuse"},
		{73, "Lord Vortech (unreleased)"},
		{74, "Blossom"},
		{75, "Bubbles"},
		{76, "Buttercup"},
		{77, "Starfire"},
		{78, "World 15 (unreleased)"},
		{79, "World 16 (unreleased)"},
		{80, "World 17 (unreleased)"},
		{81, "World 18 (unreleased)"},
		{82, "World 19 (unreleased)"},
		{83, "World 20 (unreleased)"},
		{768, "Unknown 768"},
		{769, "Supergirl Red Lantern"},
		{770, "Unknown 770"}};

	DimensionsToypadDevice::DimensionsToypadDevice()
		: Device(0x0E6F, 0x0241, 1, 2, 0)
	{
		m_IsOpened = false;
	}

	bool DimensionsToypadDevice::Open()
	{
		if (!IsOpened())
		{
			m_IsOpened = true;
		}
		return true;
	}

	void DimensionsToypadDevice::Close()
	{
		if (IsOpened())
		{
			m_IsOpened = false;
		}
	}

	bool DimensionsToypadDevice::IsOpened()
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

	Device::ReadResult DimensionsToypadDevice::Read(ReadMessage* message)
	{
		memcpy(message->data, g_dimensionstoypad.get_status().data(), message->length);
		cemuLog_log(LogType::Force, "Toypad Read Response: \n{}", HexDump(message->data, message->length));
		message->bytesRead = message->length;
		return Device::ReadResult::Success;
	}

	Device::WriteResult DimensionsToypadDevice::Write(WriteMessage* message)
	{
		cemuLog_log(LogType::Force, "Toypad Write Request: \n{}", HexDump(message->data, message->length));
		g_dimensionstoypad.send_command(message->data, message->length);
		message->bytesWritten = message->length;
		return Device::WriteResult::Success;
	}

	bool DimensionsToypadDevice::GetDescriptor(uint8 descType,
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
		*(uint8*)(currentWritePtr + 1) = 0x02;	  // bEndpointAddress
		*(uint8*)(currentWritePtr + 2) = 0x03;	  // bmAttributes
		*(uint16be*)(currentWritePtr + 3) = 0x40; // wMaxPacketSize
		*(uint8*)(currentWritePtr + 5) = 0x01;	  // bInterval
		currentWritePtr = currentWritePtr + 7;

		cemu_assert_debug((currentWritePtr - configurationDescriptor) == 0x29);

		memcpy(output, configurationDescriptor,
			   std::min<uint32>(outputMaxLength, sizeof(configurationDescriptor)));
		return true;
	}

	bool DimensionsToypadDevice::SetProtocol(uint32 ifIndex, uint32 protocol)
	{
		cemuLog_log(LogType::Force, "Toypad Protocol");
		return true;
	}

	bool DimensionsToypadDevice::SetReport(ReportMessage* message)
	{
		cemuLog_log(LogType::Force, "Toypad Report");
		return true;
	}

	std::array<uint8, 32> DimensionsUSB::get_status()
	{
		std::array<uint8, 32> response = {};

		bool responded = false;
		do
		{
			if (!m_figure_added_removed_responses.empty())
			{
				memcpy(response.data(), m_figure_added_removed_responses.front().data(),
					   0x20);
				m_figure_added_removed_responses.pop();
				responded = true;
			}
			else if (!m_queries.empty())
			{
				memcpy(response.data(), m_queries.front().data(), 0x20);
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

	void DimensionsUSB::send_command(uint8* buf, sint32 originalLength)
	{
		const uint8 command = buf[2];
		const uint8 sequence = buf[3];

		std::array<uint8, 32> q_result{};

		switch (command)
		{
		// Wake
		case 0xB0:
		{
			q_result = {0x55, 0x0e, 0x01, 0x28, 0x63, 0x29,
						0x20, 0x4c, 0x45, 0x47, 0x4f, 0x20,
						0x32, 0x30, 0x31, 0x34, 0x46};
			break;
		}
		// Seed
		case 0xB1:
		{
			g_dimensionstoypad.get_next_seed(&buf[4], sequence, q_result);
			break;
		}
		// Challenge
		case 0xB3:
		{
			g_dimensionstoypad.get_challenge_response(&buf[4], sequence, q_result);
			break;
		}
		// Color
		case 0xC0:
		// Fade
		case 0xC2:
		// Fade All
		case 0xC6:
		{
			q_result = {0x55, 0x01, sequence};
			q_result[3] = generate_checksum(q_result, 3);
			break;
		}
		// Read
		case 0xD2:
		{
			g_dimensionstoypad.query_block(buf[4], buf[5], q_result, sequence);
			break;
		}
		// Write
		case 0xD3:
		{
			g_dimensionstoypad.write_block(buf[4], buf[5], &buf[6], q_result, sequence);
			break;
		}
		// Model
		case 0xD4:
		{
			g_dimensionstoypad.get_model(&buf[4], sequence, q_result);
			break;
		}
		// Get Pad Color
		case 0xC1:
		// Flash
		case 0xC3:
		// Fade Random
		case 0xC4:
		// Flash All
		case 0xC7:
		// Color All
		case 0xC8:
		// Tag List
		case 0xD0:
		// PWD
		case 0xE1:
		// Active
		case 0xE5:
		// LEDS Query
		case 0xFF:
		{
			cemuLog_log(LogType::Force, "Unimplemented LD Function: {:x}", command);
			break;
		}
		default:
		{
			cemuLog_log(LogType::Force, "Unknown LD Function: {:x}", command);
			break;
		}
		}

		m_queries.push(q_result);
	}

	uint16 DimensionsUSB::get_figure(uint8 index)
	{
		DimensionsMini& figure = figures[index - 1];
		if (figure.id != 0)
		{
			return figure.id;
		}
		return 0;
	}

	uint16 DimensionsUSB::load_figure(const std::array<uint8, 0x2D * 0x04>& buf, std::unique_ptr<FileStream> file, uint8 pad, uint8 index)
	{
		std::lock_guard lock(m_dimensions_mutex);
		uint16 id = uint16(buf[0x0E]) << 8 | uint16(buf[0x0F]);

		DimensionsMini& figure = figures[index - 1];
		figure.dim_file = std::move(file);
		figure.id = id;
		figure.pad = pad;
		figure.index = index;
		memcpy(figure.data.data(), buf.data(), buf.size());
		std::array<uint8, 32> figure_change_response = {0x56, 0x0b, pad, 0x00, index, 0x00};
		memcpy(&figure_change_response[6], buf.data(), 7);
		figure_change_response[13] = generate_checksum(figure_change_response, 13);
		m_figure_added_removed_responses.push(figure_change_response);
		return id;
	}

	bool DimensionsUSB::remove_figure(uint8 pad, uint8 index)
	{
		std::lock_guard lock(m_dimensions_mutex);
		DimensionsMini& figure = figures[index - 1];
		if (figure.index == 255)
		{
			return false;
		}
		figure.Save();
		figure.dim_file.reset();
		figure.index = 255;
		figure.pad = 255;
		figure.id = 0;
		std::array<uint8, 32> figure_change_response = {0x56, 0x0b, pad, 0x00, index, 0x01};
		memcpy(&figure_change_response[6], figure.data.data(), 7);
		figure_change_response[13] = generate_checksum(figure_change_response, 13);
		m_figure_added_removed_responses.push(figure_change_response);
		return true;
	}

	bool DimensionsUSB::create_figure(fs::path pathName, uint16 id)
	{
		FileStream* dim_file(FileStream::createFile2(pathName));
		if (!dim_file)
		{
			return false;
		}
		std::array<uint8, 0x2D * 0x04> file_data{};
		random_uid(file_data.data());
		file_data[0x0E] = uint8((id >> 8) & 0xFF);
		file_data[0x0F] = uint8(id & 0xFF);
		if (file_data.size() != dim_file->writeData(file_data.data(), file_data.size()))
		{
			delete dim_file;
			return false;
		}
		delete dim_file;
		cemuLog_log(LogType::Force, "File data: \n{}", HexDump(file_data.data(), file_data.size()));
		return true;
	}

	void DimensionsUSB::get_next_seed(uint8* buf, uint8 sequence,
									  std::array<uint8, 32>& reply_buf)
	{
		std::array<uint8, 8> value = decrypt(buf);
		uint32 seed = uint32(value[3]) << 24 | uint32(value[2]) << 16 | uint32(value[1]) << 8 | uint32(value[0]);
		uint32 conf = uint32(value[4]) << 24 | uint32(value[5]) << 16 | uint32(value[6]) << 8 | uint32(value[7]);
		generate_seed(seed);
		std::array<uint8, 8> value_to_encrypt = {value[4], value[5], value[6], value[7], 0, 0, 0, 0};
		std::array<uint8, 8> encrypted = encrypt(value_to_encrypt.data());
		reply_buf[0] = 0x55;
		reply_buf[1] = 0x09;
		reply_buf[2] = sequence;
		memcpy(&reply_buf[3], encrypted.data(), encrypted.size());
		reply_buf[11] = generate_checksum(reply_buf, 11);
	}

	void DimensionsUSB::get_challenge_response(uint8* buf, uint8 sequence,
											   std::array<uint8, 32>& reply_buf)
	{
		std::array<uint8, 8> value = decrypt(buf);
		uint32 conf = uint32(value[0]) << 24 | uint32(value[1]) << 16 | uint32(value[2]) << 8 | uint32(value[3]);
		uint32 next_random = get_next();
		std::array<uint8, 8> value_to_encrypt = {uint8(next_random & 0xFF), uint8((next_random >> 8) & 0xFF),
												 uint8((next_random >> 16) & 0xFF), uint8((next_random >> 24) & 0xFF),
												 value[0], value[1], value[2], value[3]};
		std::array<uint8, 8> encrypted = encrypt(value_to_encrypt.data());
		reply_buf[0] = 0x55;
		reply_buf[1] = 0x09;
		reply_buf[2] = sequence;
		memcpy(&reply_buf[3], encrypted.data(), encrypted.size());
		reply_buf[11] = generate_checksum(reply_buf, 11);
	}

	void DimensionsUSB::generate_seed(uint32 seed)
	{
		random_a = 0xF1EA5EED;
		random_b = seed;
		random_c = seed;
		random_d = seed;

		for (int i = 0; i < 42; i++)
		{
			get_next();
		}
	}

	uint32 DimensionsUSB::get_next()
	{
		uint32 e = random_a - std::rotl(random_b, 21);
		random_a = random_b ^ std::rotl(random_c, 19);
		random_b = random_c + std::rotl(random_d, 6);
		random_c = random_d + e;
		random_d = e + random_a;
		return random_d;
	}

	std::array<uint8, 8> DimensionsUSB::decrypt(uint8* buf)
	{
		uint32 data_one = uint32(buf[3]) << 24 | uint32(buf[2]) << 16 | uint32(buf[1]) << 8 | uint32(buf[0]);
		uint32 data_two = uint32(buf[7]) << 24 | uint32(buf[6]) << 16 | uint32(buf[5]) << 8 | uint32(buf[4]);

		uint32 key_one = uint32(KEY[3]) << 24 | uint32(KEY[2]) << 16 | uint32(KEY[1]) << 8 | uint32(KEY[0]);
		uint32 key_two = uint32(KEY[7]) << 24 | uint32(KEY[6]) << 16 | uint32(KEY[5]) << 8 | uint32(KEY[4]);
		uint32 key_three = uint32(KEY[11]) << 24 | uint32(KEY[10]) << 16 | uint32(KEY[9]) << 8 | uint32(KEY[8]);
		uint32 key_four = uint32(KEY[15]) << 24 | uint32(KEY[14]) << 16 | uint32(KEY[13]) << 8 | uint32(KEY[12]);

		uint32 sum = 0xC6EF3720;
		uint32 delta = 0x9E3779B9;

		for (int i = 0; i < 32; i++)
		{
			data_two -= (((data_one << 4) + key_three) ^ (data_one + sum) ^ ((data_one >> 5) + key_four)) >> 0;
			data_one -= (((data_two << 4) + key_one) ^ (data_two + sum) ^ ((data_two >> 5) + key_two)) >> 0;
			sum -= delta;
			sum >>= 0;
		}

		std::array<uint8, 8> decrypted = {uint8(data_one & 0xFF), uint8((data_one >> 8) & 0xFF),
										  uint8((data_one >> 16) & 0xFF), uint8((data_one >> 24) & 0xFF),
										  uint8(data_two & 0xFF), uint8((data_two >> 8) & 0xFF),
										  uint8((data_two >> 16) & 0xFF), uint8((data_two >> 24) & 0xFF)};
		return decrypted;
	}
	std::array<uint8, 8> DimensionsUSB::encrypt(uint8* buf)
	{
		uint32 data_one = uint32(buf[3]) << 24 | uint32(buf[2]) << 16 | uint32(buf[1]) << 8 | uint32(buf[0]);
		uint32 data_two = uint32(buf[7]) << 24 | uint32(buf[6]) << 16 | uint32(buf[5]) << 8 | uint32(buf[4]);

		uint32 key_one = uint32(KEY[3]) << 24 | uint32(KEY[2]) << 16 | uint32(KEY[1]) << 8 | uint32(KEY[0]);
		uint32 key_two = uint32(KEY[7]) << 24 | uint32(KEY[6]) << 16 | uint32(KEY[5]) << 8 | uint32(KEY[4]);
		uint32 key_three = uint32(KEY[11]) << 24 | uint32(KEY[10]) << 16 | uint32(KEY[9]) << 8 | uint32(KEY[8]);
		uint32 key_four = uint32(KEY[15]) << 24 | uint32(KEY[14]) << 16 | uint32(KEY[13]) << 8 | uint32(KEY[12]);

		uint32 sum = 0;
		uint32 delta = 0x9E3779B9;

		for (int i = 0; i < 32; i++)
		{
			sum += delta;
			sum >>= 0;
			data_one += (((data_two << 4) + key_one) ^ (data_two + sum) ^ ((data_two >> 5) + key_two)) >> 0;
			data_two += (((data_one << 4) + key_three) ^ (data_one + sum) ^ ((data_one >> 5) + key_four)) >> 0;
		}

		std::array<uint8, 8> encrypted = {uint8(data_one & 0xFF), uint8((data_one >> 8) & 0xFF),
										  uint8((data_one >> 16) & 0xFF), uint8((data_one >> 24) & 0xFF),
										  uint8(data_two & 0xFF), uint8((data_two >> 8) & 0xFF),
										  uint8((data_two >> 16) & 0xFF), uint8((data_two >> 24) & 0xFF)};
		return encrypted;
	}

	DimensionsUSB::DimensionsMini&
	DimensionsUSB::get_figure_by_index(uint8 index)
	{
		for (uint8 i = 0; i < figures.size(); i++)
		{
			if (figures[i].index == index)
			{
				return figures[i];
			}
		}
		return figures[0];
	}

	void DimensionsUSB::query_block(uint8 index, uint8 page,
									std::array<uint8, 32>& reply_buf,
									uint8 sequence)
	{
		std::lock_guard lock(m_dimensions_mutex);

		DimensionsMini& figure = get_figure_by_index(index);

		reply_buf[0] = 0x55;
		reply_buf[1] = 0x12;
		reply_buf[2] = sequence;
		reply_buf[3] = 0x00;
		if (page < 45)
		{
			memcpy(&reply_buf[4], figure.data.data() + (16 * page), 16);
		}
		reply_buf[20] = generate_checksum(reply_buf, 20);
	}

	void DimensionsUSB::write_block(uint8 index, uint8 page, const uint8* to_write_buf,
									std::array<uint8, 32>& reply_buf, uint8 sequence)
	{
		std::lock_guard lock(m_dimensions_mutex);

		DimensionsMini& figure = get_figure_by_index(index);

		reply_buf[0] = 0x55;
		reply_buf[1] = 0x02;
		reply_buf[2] = sequence;
		reply_buf[3] = 0x00;
		if (figure.id != 0 && (page * 4) < 0x2D)
		{
			memcpy(figure.data.data() + (page * 4), to_write_buf, 4);
			figure.Save();
		}
		reply_buf[4] = generate_checksum(reply_buf, 4);
	}

	void DimensionsUSB::get_model(uint8* buf, uint8 sequence,
								  std::array<uint8, 32>& reply_buf)
	{
		std::array<uint8, 8> value = decrypt(buf);
		uint8 index = value[0];
		uint32 conf = uint32(value[4]) << 24 | uint32(value[5]) << 16 | uint32(value[6]) << 8 | uint32(value[7]);
		DimensionsMini& figure = get_figure_by_index(index);
		std::array<uint8, 8> value_to_encrypt = {uint8(figure.id & 0xFF), uint8((figure.id >> 8) & 0xFF),
												 uint8((figure.id >> 16) & 0xFF), uint8((figure.id >> 24) & 0xFF),
												 value[4], value[5], value[6], value[7]};
		std::array<uint8, 8> encrypted = encrypt(value_to_encrypt.data());
		reply_buf[0] = 0x55;
		reply_buf[1] = 0x0a;
		reply_buf[2] = sequence;
		reply_buf[3] = 0x00;
		memcpy(&reply_buf[4], encrypted.data(), encrypted.size());
		reply_buf[12] = generate_checksum(reply_buf, 12);
	}

	void DimensionsUSB::random_uid(uint8* uid_buffer)
	{
		uid_buffer[0] = 0x04;
		uid_buffer[6] = 0x80;

		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dist(0, 255);

		uid_buffer[1] = dist(mt);
		uid_buffer[2] = dist(mt);
		uid_buffer[3] = dist(mt);
		uid_buffer[4] = dist(mt);
		uid_buffer[5] = dist(mt);
	}

	uint8 DimensionsUSB::generate_checksum(const std::array<uint8, 32>& data,
										   int num_of_bytes) const
	{
		int checksum = 0;
		for (int i = 0; i < num_of_bytes; i++)
		{
			checksum += data[i];
		}
		return (checksum & 0xFF);
	}

	void DimensionsUSB::DimensionsMini::Save()
	{
		if (!dim_file)
			return;

		dim_file->SetPosition(0);
		dim_file->writeData(data.data(), data.size());
	}
} // namespace nsyshid