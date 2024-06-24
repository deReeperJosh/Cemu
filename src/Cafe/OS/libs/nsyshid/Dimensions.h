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
		void send_command(uint8* buf, sint32 originalLength);
		std::array<uint8, 32> get_status();

		void get_next_seed(uint8* buf, uint8 sequence,
						   std::array<uint8, 32>& reply_buf);
		void generate_seed(uint32 seed);
		void get_challenge_response(uint8* buf, uint8 sequence,
									std::array<uint8, 32>& reply_buf);
		uint32 get_next();

	  protected:
		std::mutex m_infinity_mutex;

	  private:
		uint8 generate_checksum(const std::array<uint8, 32>& data,
								int num_of_bytes) const;
		std::array<uint8, 8> decrypt(uint8* buf);
		std::array<uint8, 8> encrypt(uint8* buf);

		uint32 random_a;
		uint32 random_b;
		uint32 random_c;
		uint32 random_d;

		std::queue<std::array<uint8, 32>> m_queries;
	};
	extern DimensionsUSB g_dimensionstoypad;

} // namespace nsyshid