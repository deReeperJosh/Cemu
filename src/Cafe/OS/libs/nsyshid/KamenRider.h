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
						   uint8 lang,
						   uint8* output,
						   uint32 outputMaxLength) override;

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
			std::unique_ptr<FileStream> dimFile;
			std::array<uint8, 0x2D * 0x04> data{};
			std::array<uint8, 7> uid{};
			bool present = false;
			void Save();
		};

		void SendCommand(std::span<const uint8> buf);
		std::array<uint8, 64> GetStatus();

	  protected:
		std::mutex m_kamenRiderMutex;
		std::array<RiderFigure, 7> m_figures{};

	  private:
		void GetListTags(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence);
		void QueryBlock(std::array<uint8, 64>& replyBuf, uint8 command, uint8 sequence,
						const std::array<uint8, 7> uid, uint8 block, uint8 line);
		uint8 GenerateChecksum(const std::array<uint8, 64>& data,
							   int numOfBytes) const;
		std::optional<RiderFigure&> GetFigureByUID(const std::array<uint8, 7> uid);

		std::queue<std::array<uint8, 64>> m_figureAddedRemovedResponses;
		std::queue<std::array<uint8, 64>> m_queries;
	};
	extern RideGateUSB g_kamenridegate;
} // namespace nsyshid