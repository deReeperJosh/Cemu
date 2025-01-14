#pragma once

#include <mutex>

#include "nsyshid.h"
#include "Backend.h"

#include "Common/FileStream.h"

namespace nsyshid
{
	class KamenRiderGateDevice final : public Device
	{
	  public:
		KamenRiderGateDevice();
		~KamenRiderGateDevice() = default;

		bool Open() override;

		void Close() override;

		bool IsOpened() override;

		ReadResult Read(ReadMessage* message) override;

		WriteResult Write(WriteMessage* message) override;

		bool GetDescriptor(uint8 descType,
						   uint8 descIndex,
						   uint16 lang,
						   uint8* output,
						   uint32 outputMaxLength) override;

		bool SetIdle(uint8 ifIndex,
					 uint8 reportId,
					 uint8 duration) override;

		bool SetProtocol(uint8 ifIndex, uint8 protocol) override;

		bool SetReport(ReportMessage* message) override;

	  private:
		bool m_IsOpened;
	};

	class RideGateUSB
	{
	  public:
		struct RiderFigure final
		{
			std::unique_ptr<FileStream> kamenFile;
			std::array<uint8, 0x14 * 0x10> data{};
			std::array<uint8, 7> uid{};
			bool present = false;
			void Save();
		};

		void SendCommand(std::span<const uint8> buf, const uint8 length);
		std::array<uint8, 64> GetStatus();

		void GetListTags(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence);
		void QueryBlock(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence,
						const uint8* uid, uint8 block, uint8 line);
		void WriteBlock(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence,
						const uint8* uid, uint8 block, uint8 line, const uint8* toWriteBuf);
		void GetBlankResponse(uint8 command, uint8 sequence, std::array<uint8, 64>& replyBuf);

		bool RemoveFigure(uint8 index);
		uint8 LoadFigure(const std::array<uint8, 0x14 * 0x10>& buf, std::unique_ptr<FileStream> file);
		bool CreateFigure(fs::path pathName, uint8 type, uint8 id);

		std::string FindFigure(uint8 type, uint8 id);

	  protected:
		std::mutex m_kamenRiderMutex;
		std::array<RiderFigure, 8> m_figures{};

	  private:
		uint8 GenerateChecksum(const std::array<uint8, 64>& data,
							   int numOfBytes) const;
		RiderFigure& GetFigureByUID(const std::array<uint8, 7> uid);

		std::queue<std::array<uint8, 64>> m_figureAddedRemovedResponses;
		std::queue<std::array<uint8, 64>> m_queries;

		bool m_isAwake = false;
	};
	extern RideGateUSB g_kamenridegate;
} // namespace nsyshid