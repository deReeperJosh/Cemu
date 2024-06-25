#include <mutex>

#include "nsyshid.h"
#include "Backend.h"

namespace nsyshid
{
	class DimensionsToypadDevice final : public Device {
	  public:
		DimensionsToypadDevice();
		~DimensionsToypadDevice() = default;

		bool Open() override;

		void Close() override;

		bool IsOpened() override;

		ReadResult Read(ReadMessage* message) override;

		WriteResult Write(WriteMessage* message) override;

		bool GetDescriptor(uint8 descType,
						   uint8 descIndex,
						   uint8 lang,
						   uint8* output,
						   uint32 outputMaxLength) override;

		bool SetProtocol(uint32 ifIndex, uint32 protocol) override;

		bool SetReport(ReportMessage* message) override;

	  private:
		bool m_IsOpened;
	};

	class DimensionsUSB {
	  public:
		struct DimensionsMini final
		{
			std::FILE* dim_file;
			std::array<uint8, 0x2D * 0x04> data{};
			uint8 index = 255;
			uint8 pad = 255;
			uint8 id = 0;
			void Save();
		};

		void send_command(uint8* buf, sint32 originalLength);
		std::array<uint8, 32> get_status();

		void get_next_seed(uint8* buf, uint8 sequence,
						   std::array<uint8, 32>& reply_buf);
		void generate_seed(uint32 seed);
		void get_challenge_response(uint8* buf, uint8 sequence,
									std::array<uint8, 32>& reply_buf);
		void query_block(uint8 index, uint8 page, std::array<uint8, 32>& reply_buf,
						 uint8 sequence);
		void get_model(uint8* buf, uint8 sequence,
					   std::array<uint8, 32>& reply_buf);

		uint16 get_figure(uint8 index);
		bool remove_figure(uint8 pad, uint8 index);
		uint16 load_figure(const std::array<uint8, 0x2D * 0x04>& buf, std::FILE* file, uint8 pad, uint8 index);
		bool create_figure(const std::string& file_path, uint16 id);

	  protected:
		std::mutex m_dimensions_mutex;
		std::array<DimensionsMini, 7> figures;

	  private:
		void random_uid(uint8* uid_buffer);
		uint8 generate_checksum(const std::array<uint8, 32>& data,
								int num_of_bytes) const;
		std::array<uint8, 8> decrypt(uint8* buf);
		std::array<uint8, 8> encrypt(uint8* buf);
		uint32 get_next();
		DimensionsMini& get_figure_by_index(uint8 index);

		uint32 random_a;
		uint32 random_b;
		uint32 random_c;
		uint32 random_d;

		std::array<uint8, 7> batman_uid = {};
		std::array<uint8, 7> gandalf_uid = {};
		std::array<uint8, 7> wyldstyle_uid = {};

		std::queue<std::array<uint8, 32>> m_figure_added_removed_responses;
		std::queue<std::array<uint8, 32>> m_queries;
	};
	extern DimensionsUSB g_dimensionstoypad;

} // namespace nsyshid