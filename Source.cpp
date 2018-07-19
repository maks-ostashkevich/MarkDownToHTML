#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm>
#include <string>
#include <exception>

#include <queue>
#include <tuple>
#include <stack>

class Reader
{
public:
	Reader(std::string fileName) : 
		fileStream(fileName), fileName(fileName), complete(false)
	{
		std::string line;
		if (!getline(fileStream, line))
		{
			complete = true;
			currLine = "EOF";
		}
		else
		{
			currLine = line;
		}
	}

	std::string getNext()
	{
		if (complete)
		{
			throw std::exception();
		}

		std::string line;
		if (!getline(fileStream, line))
		{
			complete = true;
		}
		if (!complete)
		{
			std::string retLine = currLine;
			currLine = line;
			return retLine;
		}
		else
		{
			std::string retLine = currLine;
			currLine = "EOF";
			return retLine;
		}
	}

	bool isComplete()
	{
		return complete;
	}
private:
	std::string currLine;
	bool complete;
	std::string fileName;
	std::ifstream fileStream;
};

class Parser
{
public:
	Parser(Reader& reader) : reader(reader), listOpened(false)
	{

	}

	std::string parse(std::string line)
	{
		line = trim(line);

		bool lineIsEmpty = false;
		if (line.empty() == true)
		{
			lineIsEmpty = true;
		}

		bool isHeader = false;
		std::string headersString = "";
		bool islistItem = false;
		if (line.substr(0, 2) == "* ")
		{
			line = line.substr(2, line.length() - 2);
			islistItem = true;
		}

		if (line.size() >= 2 && line.substr(0, 2) == "# ")
		{
			isHeader = true;
			headersString = "<h1></h1>";
			line = line.substr(2, line.length() - 2);
		}
		else if (line.size() >= 3 && line.substr(0, 3) == "## ")
		{
			isHeader = true;
			headersString = "<h2></h2>";
			line = line.substr(3, line.length() - 3);
		}
		else if (line.size() >= 4 && line.substr(0, 4) == "### ")
		{
			isHeader = true;
			headersString = "<h3></h3>";
			line = line.substr(4, line.length() - 4);
		}
		else if (line.size() >= 5 && line.substr(0, 5) == "#### ")
		{
			isHeader = true;
			headersString = "<h4></h4>";
			line = line.substr(5, line.length() - 5);
		}
		else if (line.size() >= 6 && line.substr(0, 6) == "##### ")
		{
			isHeader = true;
			headersString = "<h5></h5>";
			line = line.substr(6, line.length() - 6);
		}
		else if (line.size() >= 7 && line.substr(0, 7) == "###### ")
		{
			isHeader = true;
			headersString = "<h6></h6>";
			line = line.substr(7, line.length() - 7);
		}
		
		std::queue<std::tuple<int, std::string, std::string, bool>> mods;

		for (int curr = 0;;)
		{
			int starPos = line.find("*", curr);
			if (starPos == std::string::npos)
			{
				if (std::get<1>(boldOpened(mods)) != "NO_ITEM" && std::get<1>(italicOpened(mods)) == "NO_ITEM" ||
					std::get<1>(boldOpened(mods)) == "NO_ITEM" && std::get<1>(italicOpened(mods)) != "NO_ITEM")
				{
					removeLastOpened(mods);
				}
				else
				{
					if (std::get<1>(boldOpened(mods)) == "NO_ITEM" && std::get<1>(italicOpened(mods)) == "NO_ITEM")
					{
						break;
					}

					if (std::get<0>(boldOpened(mods)) < std::get<0>(italicOpened(mods)))
					{
						// выгр пока не найдём (последний) открытый болд и его надо разбить и открыть для италика, а италик закрыть
						std::queue<std::tuple<int, std::string, std::string, bool>> tempMods;
						while (!(std::get<1>(mods.front()) == "**" && std::get<3>(mods.front()) == true) && mods.empty() == false)
						{
							tempMods.push(mods.front());
							mods.pop();
						}

						std::tuple<int, std::string, std::string, bool> changeLastOpenedBold =
						{ std::get<0>(mods.front()), "*", "opening",  false };
						tempMods.push(changeLastOpenedBold);
						mods.pop();

						while (!(std::get<1>(mods.front()) == "*" && std::get<3>(mods.front()) == true) && mods.empty() == false)
						{
							tempMods.push(mods.front());
							mods.pop();
						}

						std::tuple<int, std::string, std::string, bool> changeLastOpenedItalic =
						{ std::get<0>(mods.front()), "*", "closing",  false };
						tempMods.push(changeLastOpenedItalic);
						mods.pop();

						while (mods.empty() == false)
						{
							tempMods.push(mods.front());
							mods.pop();
						}

						while (tempMods.empty() == false)
						{
							mods.push(tempMods.front());
							tempMods.pop();
						}
					}
					else
					{
						// выгр пока не найдём (последний) открытый болд и его надо разбить и замкнуть италик
						std::queue<std::tuple<int, std::string, std::string, bool>> tempMods;
						while (!(std::get<1>(mods.front()) == "**" && std::get<3>(mods.front()) == true) && mods.empty() == false)
						{
							tempMods.push(mods.front());
							mods.pop();
						}

						std::tuple<int, std::string, std::string, bool> changeLastOpenedBold =
						{ std::get<0>(mods.front()), "*", "closing",  false };
						tempMods.push(changeLastOpenedBold);
						mods.pop();

						while (mods.empty() == false)
						{
							tempMods.push(mods.front());
							mods.pop();
						}

						while (tempMods.empty() == false)
						{
							mods.push(tempMods.front());
							tempMods.pop();
						}
					}
				}

				break;
			}

			if (std::get<1>(lastOpened(mods)) == "NO_ITEM")
			{
				if (starPos < line.length() - 1 && line[starPos + 1] == '*')
				{
					mods.push({ starPos, "**", "opening", true });
					curr = starPos + 2;
				}
				else
				{
					mods.push({ starPos, "*", "opening", true });
					curr = starPos + 1;
				}
			}
			else
			{
				// p. a. !
				if (std::get<1>(lastOpened(mods)) == "*")
					// case for italic
				{
					// close last opened
					// add {starPos, "*", false}
					// if all closed, then form the part of string

					if (starPos < line.length() - 2 && line[starPos + 1] == '*' && line[starPos + 2] != '*' ||
						starPos == line.length() - 2 && line[starPos + 1] == '*')
					{
						if (std::get<1>(boldOpened(mods)) != "NO_ITEM")
						{
							removeLastOpened(mods); // 
							closeLastOpenedBold(mods);
							mods.push({ starPos, "**", "closing", false });
							curr = starPos + 2;
						}
						else
						{
							mods.push({ starPos, "**", "opening", true });
							curr = starPos + 2;
						}
					}
					else
					{
						closeLastOpened(mods);
						mods.push({ starPos, "*", "closing", false });
						curr = starPos + 1;
					}
				}
				else
					// case for bold
				{
					// если есть * после текущей *, тогда отцепляем её,
					// иначе, расцепляем последний открытый с возвращением правой звезды из двух в текст
					if (starPos < line.length() - 1 && line[starPos + 1] == '*')
					{
						closeLastOpened(mods);
						mods.push({ starPos, "**", "closing", false });
						curr = starPos + 2; // -> length
					}
					else
					{
						if (std::get<1>(italicOpened(mods)) != "NO_ITEM")
						{
							removeLastOpened(mods); // 
							closeLastOpenedItalic(mods);
							mods.push({ starPos, "*", "closing", false });
							curr = starPos + 1;
						}
						else
						{
							mods.push({ starPos, "*", "opening", true });
							curr = starPos + 1;
						}
					}
				}
			}
		}

		line = formStringFromQueue(line, mods);

		std::string formedString = line;

		if (lineIsEmpty)
		{
			formedString = "";

			if (listOpened)
			{
				listOpened = false;
				return "</ul>\n" + formedString;
			}

			return formedString;
		}
		
		if (islistItem)
		{
			if (listOpened)
			{
				if (isHeader)
					formedString = headersString.insert(4, formedString);
				formedString = "<li>" + formedString + "</li>";
				if (reader.isComplete())
				{
					formedString += "\n</ul>";
				}
				return formedString;
			}
			else
			{
				if (isHeader)
					formedString = headersString.insert(4, formedString);
				formedString = "<li>" + formedString + "</li>";
				formedString = "<ul>\n" + formedString;
				listOpened = true;
				return formedString;
			}
		}
		if (listOpened)
		{
			listOpened = false;
			if (isHeader)
				formedString = headersString.insert(4, formedString);
			return "</ul>\n" + formedString;
		}
		return formedString;
	}

	std::string getNext()
	{
		std::string curr = reader.getNext();
		return parse(curr);
	}

	bool isComplete()
	{
		return reader.isComplete();
	}

private:
	std::string trim(const std::string& str,
		const std::string& whitespace = " \t")
	{
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return ""; // no content

		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	std::tuple<int, std::string, std::string, bool> lastOpened(std::queue<std::tuple<int, std::string, std::string, bool>> mods) // std::string
	{
		std::tuple<int, std::string, std::string, bool> lOpened = { -1, "NO_ITEM", "UNKNOWN", false };
		// std::queue<std::tuple<int, std::string, bool>> saveMods;
		while (mods.empty() == false)
		{
			if (std::get<3>(mods.front()) == true)
			{
				lOpened = mods.front(); // std::get<1>(mods.front());
			}
			// saveMods.push(mods.pop());
			mods.pop();
		}

		return lOpened; // "*"; // stab
	}
	std::tuple<int, std::string, std::string, bool> boldOpened(std::queue<std::tuple<int, std::string, std::string, bool>> mods) // std::string
	{
		std::tuple<int, std::string, std::string, bool> lOpened = { -1, "NO_ITEM", "UNKNOWN", false };
		// std::queue<std::tuple<int, std::string, bool>> saveMods;
		while (mods.empty() == false)
		{
			if (std::get<3>(mods.front()) == true && std::get<1>(mods.front()) == "**")
			{
				lOpened = mods.front(); // std::get<1>(mods.front());
			}
			// saveMods.push(mods.pop());
			mods.pop();
		}

		return lOpened; // "*"; // stab
	}
	std::tuple<int, std::string, std::string, bool> italicOpened(std::queue<std::tuple<int, std::string, std::string, bool>> mods) // std::string
	{
		std::tuple<int, std::string, std::string, bool> lOpened = { -1, "NO_ITEM", "UNKNOWN", false };
		// std::queue<std::tuple<int, std::string, bool>> saveMods;
		while (mods.empty() == false)
		{
			if (std::get<3>(mods.front()) == true && std::get<1>(mods.front()) == "*")
			{
				lOpened = mods.front(); // std::get<1>(mods.front());
			}
			// saveMods.push(mods.pop());
			mods.pop();
		}

		return lOpened; // "*"; // stab
	}
	// => getLastOpened() with pointers to the corresp. functions with bool

	void closeLastOpened(std::queue<std::tuple<int, std::string, std::string, bool>>& mods)
	{
		std::stack<std::tuple<int, std::string, std::string, bool>> modsStack;

		while (mods.empty() == false)
		{
			// std::cout << mods.pop() << std::endl;
			modsStack.push(mods.front()); // {0, "", false}
			mods.pop();
		}

		std::stack<std::tuple<int, std::string, std::string, bool>> reversedModsStack;
		while (std::get<3>(modsStack.top()) == false)
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		reversedModsStack.push({ std::get<0>(modsStack.top()), std::get<1>(modsStack.top()), std::get<2>(modsStack.top()), false });
		modsStack.pop();

		while (modsStack.empty() == false)
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		while (reversedModsStack.empty() == false)
		{
			// reversedModsStack.push(modsStack.top()); // mods
			// modsStack.pop();
			mods.push(reversedModsStack.top());
			reversedModsStack.pop();
		}
	}
	void closeLastOpenedBold(std::queue<std::tuple<int, std::string, std::string, bool>>& mods)
	{
		std::stack<std::tuple<int, std::string, std::string, bool>> modsStack;

		while (mods.empty() == false)
		{
			// std::cout << mods.pop() << std::endl;
			modsStack.push(mods.front()); // {0, "", false}
			mods.pop();
		}

		std::stack<std::tuple<int, std::string, std::string, bool>> reversedModsStack;
		while (std::get<3>(modsStack.top()) == false && std::get<1>(modsStack.top()) != "**")
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		reversedModsStack.push({ std::get<0>(modsStack.top()), std::get<1>(modsStack.top()), std::get<2>(modsStack.top()), false });
		modsStack.pop();

		while (modsStack.empty() == false)
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		while (reversedModsStack.empty() == false)
		{
			// reversedModsStack.push(modsStack.top()); // mods
			// modsStack.pop();
			mods.push(reversedModsStack.top());
			reversedModsStack.pop();
		}
	}
	void closeLastOpenedItalic(std::queue<std::tuple<int, std::string, std::string, bool>>& mods)
	{
		std::stack<std::tuple<int, std::string, std::string, bool>> modsStack;

		while (mods.empty() == false)
		{
			// std::cout << mods.pop() << std::endl;
			modsStack.push(mods.front()); // {0, "", false}
			mods.pop();
		}

		std::stack<std::tuple<int, std::string, std::string, bool>> reversedModsStack;
		while (std::get<3>(modsStack.top()) == false && std::get<1>(modsStack.top()) != "*")
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		reversedModsStack.push({ std::get<0>(modsStack.top()), std::get<1>(modsStack.top()), std::get<2>(modsStack.top()), false });
		modsStack.pop();

		while (modsStack.empty() == false)
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		while (reversedModsStack.empty() == false)
		{
			// reversedModsStack.push(modsStack.top()); // mods
			// modsStack.pop();
			mods.push(reversedModsStack.top());
			reversedModsStack.pop();
		}
	}
	// => getLastOpened() with pointers to the corresp. functions with bool

	void removeLastOpened(std::queue<std::tuple<int, std::string, std::string, bool>>& mods)
	{
		std::stack<std::tuple<int, std::string, std::string, bool>> modsStack;

		while (mods.empty() == false)
		{
			// std::cout << mods.pop() << std::endl;
			modsStack.push(mods.front()); // {0, "", false}
			mods.pop();
		}

		std::stack<std::tuple<int, std::string, std::string, bool>> reversedModsStack;
		while (std::get<3>(modsStack.top()) == false)
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		// reversedModsStack.push({ std::get<0>(modsStack.top()), std::get<1>(modsStack.top()), std::get<2>(modsStack.top()), false });
		modsStack.pop();

		while (modsStack.empty() == false)
		{
			reversedModsStack.push(modsStack.top()); // mods
			modsStack.pop();
		}

		while (reversedModsStack.empty() == false)
		{
			// reversedModsStack.push(modsStack.top()); // mods
			// modsStack.pop();
			mods.push(reversedModsStack.top());
			reversedModsStack.pop();
		}
	}
	bool closed(std::queue<std::tuple<int, std::string, std::string, bool>>& mods)
	{
		// cond - ?
		return std::get<1>(mods.front()) == std::get<1>(mods.back());
	}

	std::string evaluateLinks(std::string line)
	{
		// пока есть ](
		// найти текущий ](
		// найти к нему [ ближайший слева,
		// найти к нему ) ближайший справа
		// чего-то нет - стоп
		// если есть, то замена

		// int curr = 0;
		int twoSignsPos = line.find("](", 0); // curr
		while (twoSignsPos != std::string::npos)
		{
			int leftSqParPos = line.substr(0, twoSignsPos).find_last_of("[");
			if (leftSqParPos == std::string::npos) // > twoSignsPos)
			{
				break;
			}
			int rightParPos = line.find(")", twoSignsPos + 2);
			if (rightParPos == std::string::npos)
			{
				break;
			}

			std::string linkInserted = "<a href=\"" + line.substr(twoSignsPos + 2, rightParPos - twoSignsPos - 2)
				+ "\">" + line.substr(leftSqParPos + 1, twoSignsPos - leftSqParPos - 1) + "</a>";

			line = line.substr(0, leftSqParPos) + linkInserted + line.substr(rightParPos + 1, line.length() - rightParPos);

			twoSignsPos = line.find("](", 0); // curr
		}
		return line;
	}
	
	std::string formStringFromQueue(std::string line, std::queue<std::tuple<int, std::string, std::string, bool>> mods)
	{
		std::string formedString = "";

		if (mods.empty() == true)
		{
			return evaluateLinks(line); //  formedString;
		}

		formedString += line.substr(0, std::get<0>(mods.front()));

		while (mods.empty() == false)
		{
			if (std::get<1>(mods.front()) == "*")
			{
				if (std::get<2>(mods.front()) == "opening")
				{
					formedString += "<em>";
				}
				else
				{
					formedString += "</em>";
				}
			}
			else if (std::get<1>(mods.front()) == "**")
			{
				if (std::get<2>(mods.front()) == "opening")
				{
					formedString += "<strong>";
				}
				else
				{
					formedString += "</strong>";
				}
			}

			// int from = std::get<0>(mods.front());
			std::tuple<int, std::string, std::string, bool> from = mods.front();
			mods.pop();

			if (mods.empty() == false)
			{
				if (std::get<1>(from) == "*")
				{
					if (std::get<0>(from) + 1 < line.length())
					{
						// std::cout << std::get<0>(mods.front()) << std::endl;
						// std::cout << line.substr(std::get<0>(from) + 1, std::get<0>(mods.front()) - std::get<0>(from) - 1) << std::endl;
						formedString += evaluateLinks(line.substr(std::get<0>(from) + 1, std::get<0>(mods.front()) - std::get<0>(from) - 1));
					}
				}
				else
				{
					if (std::get<0>(from) + 2 < line.length())
					{
						// std::cout << std::get<0>(mods.front()) << std::endl;
						// std::cout << line.substr(std::get<0>(from) + 2, std::get<0>(mods.front()) - std::get<0>(from) - 2) << std::endl;
						formedString += evaluateLinks(line.substr(std::get<0>(from) + 2, std::get<0>(mods.front()) - std::get<0>(from) - 2));
					}
				}
			}
			else
			{
				if (std::get<1>(from) == "*")
				{
					if (std::get<0>(from) + 1 < line.length())
					{
						std::cout << line.substr(std::get<0>(from) + 1, line.length() - std::get<0>(from) - 1) << std::endl;
						formedString += evaluateLinks(line.substr(std::get<0>(from) + 1, line.length() - std::get<0>(from) - 1));
					}
				}
				else
				{
					if (std::get<0>(from) + 2 < line.length())
					{
						std::cout << line.substr(std::get<0>(from) + 2, line.length() - std::get<0>(from) - 2) << std::endl;
						formedString += evaluateLinks(line.substr(std::get<0>(from) + 2, line.length() - std::get<0>(from) - 2));
					}
				}
			}
		}
		return formedString;
	}

	bool listOpened;

	Reader& reader;
};

class Writer
{
public:
	Writer(Parser& parser, std::string output) :
		parser(parser), outFile(output)
	{
		
	}

	void writeNext()
	{
		if (!parser.isComplete())
		{
			std::string curr = parser.getNext();
			std::cout << curr << std::endl;
			std::cout << std::endl;
			outFile << curr << std::endl;
		}
	}

	bool isComplete()
	{
		return parser.isComplete();
	}

private:
	Parser& parser;
	std::ofstream outFile;
};

int main()
{
	Reader reader("file.txt");
	Parser parser(reader);
	Writer writer(parser, "outfile.txt");

	while (!writer.isComplete())
	{
		writer.writeNext();
	}
}