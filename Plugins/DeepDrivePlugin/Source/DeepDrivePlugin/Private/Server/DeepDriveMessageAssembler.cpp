// Fill out your copyright notice in the Description page of Project Settings.

#include "DeepDrivePluginPrivatePCH.h"
#include "DeepDriveMessageAssembler.h"
#include "Public/Server/Messages/DeepDriveServerMessageHeader.h"

DeepDriveMessageAssembler::DeepDriveMessageAssembler()
{
	resize(16 * 1024);
}

DeepDriveMessageAssembler::~DeepDriveMessageAssembler()
{
}


void DeepDriveMessageAssembler::add(const uint8 *data, uint32 numBytes)
{
	if (resize(numBytes))
	{
		FMemory::BigBlockMemcpy(m_MessageBuffer + m_curWritePos, data, numBytes);
		m_curWritePos += numBytes;

		(void) new (m_MessageBuffer + m_curWritePos) deepdrive::server::MessageHeader(deepdrive::server::MessageId::Undefined, 0);

		checkForMessage();
	}
}

void DeepDriveMessageAssembler::checkForMessage()
{
	deepdrive::server::MessageHeader *msg = reinterpret_cast<deepdrive::server::MessageHeader*> (m_MessageBuffer);

	uint32 readPos = 0;

	while (msg->message_size && readPos + msg->message_size <= m_curWritePos)
	{
		m_HandleMessage.Execute(*(msg));

		readPos += msg->message_size;
		msg = reinterpret_cast<deepdrive::server::MessageHeader*> (m_MessageBuffer + readPos);
	}

	if (readPos > 0)
	{
		FMemory::BigBlockMemcpy(m_MessageBuffer, m_MessageBuffer + readPos, readPos);
		m_curWritePos -= readPos;
	}
}

bool DeepDriveMessageAssembler::resize(uint32 numBytes)
{
	uint32 newSize = m_curWritePos + numBytes;
	if (newSize > m_curBufferSize)
	{
		m_MessageBuffer = reinterpret_cast<uint8*> (FMemory::Realloc(m_MessageBuffer, newSize + sizeof(deepdrive::server::MessageHeader)));	// leave head room for terminating header
		m_curBufferSize = newSize;
	}
	return m_MessageBuffer != 0;
}
