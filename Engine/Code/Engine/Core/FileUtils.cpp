#include "Engine/Core/FileUtils.hpp"
#include <fstream>

bool FileReadToBuffer(std::vector<uint8_t>& outBuffer, std::string const& filename)
{
	std::ifstream file(filename, std::ifstream::binary);

	if (file)
	{
		file.seekg(0, file.end);
		int fileLength = (int) file.tellg();
		file.seekg(0, file.beg);

		int bufferLength = fileLength + 1;
		char* charBuffer = new char[bufferLength];

		file.read(charBuffer, fileLength);
		charBuffer[fileLength] = '\0';

		outBuffer.resize(bufferLength);
		for (int byteIndex = 0; byteIndex < bufferLength; byteIndex++)
		{
			outBuffer[byteIndex] = (uint8_t) charBuffer[byteIndex];
		}

		delete[] charBuffer;
		return true;
	}

	return false;
}


bool FileReadToString(std::string& outString, std::string const& filename)
{
	std::vector<uint8_t> outBuffer;
	bool success = FileReadToBuffer(outBuffer, filename);
	if ((int) outBuffer.size() > 0)
	{
		outString = (char*) outBuffer.data();
	}
	return success;
// 	if (outBuffer.size() > 0)
// 	{
// 		outString = (char*) outBuffer.data();
// 		return true;
// 	}
// 	else
// 	{
// 		return false;
// 	}
}


bool WriteBufferToFile(std::vector<uint8_t> const& buffer, std::string const& fileName)
{
	std::ofstream file(fileName, std::ios::binary);
	if (file)
	{
		for (int byteIndex = 0; byteIndex < buffer.size(); byteIndex++)
		{
			file.put((unsigned char) buffer[byteIndex]);
		}
		//file.write((char*) buffer.data(), (int) buffer.size());
	}

	return false;
}
