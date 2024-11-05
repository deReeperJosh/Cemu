#pragma once

#include <mutex>

#include "nsyshid.h"
#include "Backend.h"

namespace nsyshid
{
    class KamenRiderGateDevice final : public Device {
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
}