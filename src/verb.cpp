// User defined libs
#include "verb.hpp"
#include "utility.hpp"
#include "log.hpp"
#include "textdata.hpp"

#define VERB_SIZE 32  // 32 default
#define VERB_SIZE_HYPHEN 4
#define VERB_ROW 26
#define VERB_COL 27
#define SIZE_INCREASE_COEFFICIENT 4 // 4 default
#define VERB_LINE_LENGHT 100

namespace verb {
	VerbHandler::VerbHandler() {
		buffer = new VerbIndexList[VERB_ROW * VERB_COL]; // buffer[row * 27 + column]
		irrVerbCollection.amount = 0;
		irrVerbCollection.capacity = 0;
		irrVerbCollection.verbs = nullptr;

		// Initialize Indexer
		for (int row = 0; row < VERB_ROW; row++) {
			for (int col = 0; col < VERB_COL; col++) {
				buffer[row * VERB_COL + col].verbs = nullptr;
				buffer[row * VERB_COL + col].verbAmount = 0;
				buffer[row * VERB_COL + col].verbCapacity = 0;
				buffer[row * VERB_COL + col].indicator[0] = 'a' + row;
				if (col == 26)
					buffer[row * VERB_COL + col].indicator[1] = '-';
				else
					buffer[row * VERB_COL + col].indicator[1] = 'a' + col;
			}
		}
	}

	VerbHandler::~VerbHandler() {
		// Delete each verb heap
		for (int row = 0; row < VERB_ROW; row++) {
			for (int col = 0; col < VERB_COL; col++) {
				if (buffer[row * VERB_COL + col].verbs != nullptr)
					delete[] buffer[row * VERB_COL + col].verbs;
			}
		}
		// Delete container
		delete[] buffer;
		// Delete irregular verb collections
		if (irrVerbCollection.verbs != nullptr)
			delete[] irrVerbCollection.verbs;
	}

	int8 VerbHandler::DeleteAll() {
		// 0 : deleted successfully
		// Delete each verb heap
		for (int row = 0; row < VERB_ROW; row++) {
			for (int col = 0; col < VERB_COL; col++) {
				if (buffer[row * VERB_COL + col].verbs != nullptr) {
					delete[] buffer[row * VERB_COL + col].verbs;

					buffer[row * VERB_COL + col].verbAmount = 0;
					buffer[row * VERB_COL + col].verbCapacity = 0;
					buffer[row * VERB_COL + col].verbs = nullptr;
				}
			}
		}

		// Delete Irregular records
		for (int i = 0; i < irrVerbCollection.amount; i++) {
			delete[] irrVerbCollection.verbs;
			irrVerbCollection.verbs = nullptr;
			irrVerbCollection.amount = 0;
			irrVerbCollection.capacity = 0;
		}
		return 0;
	}


	void VerbHandler::MultipleVerbAdder(const char& file) { // This function add multiple verbs to ram
		uint16 _successfull = 0;
		uint16 _nonAlphabetic = 0;
		uint16 _charExceedsSize = 0;
		uint16 _charSizeSmaller2 = 0;
		uint16 _notUnique = 0;

		uint16 _emptyLine = 0;



#define CAPACITY_OF_LINES 300
		MyString nonAlphabeticLines(new char[CAPACITY_OF_LINES], CAPACITY_OF_LINES);
		MyString exceedsSizeLines(new char[CAPACITY_OF_LINES], CAPACITY_OF_LINES);
		MyString sizeSmaller2CharLines(new char[CAPACITY_OF_LINES], CAPACITY_OF_LINES);
		MyString nonUniqueLines(new char[CAPACITY_OF_LINES], CAPACITY_OF_LINES);
		MyString emptyLines(new char[CAPACITY_OF_LINES], CAPACITY_OF_LINES);
#undef CAPACITY_OF_LINES

		uint16 lineLenght = 0;

		auto findNextLine = [&](const char& line) {
			uint16 i = 0;
			while ((&line)[i] != '\n' && (&line)[i] != '\0') {
				i++;
			}
			lineLenght = i;
		};

		uint32 i = -1;
		uint32 lineNumber = 1;
		char temp[15];
		do { // Read line by line
			i++; // Increment
			switch (AddNewVerb((&file)[i])) {
			case 1: // The verb Successfully added
				_successfull++;
				break;
			case -1: // There is some charachter which is not alphabetic
				_nonAlphabetic++;
				nonAlphabeticLines.AddToEnd(*temp, IntToStr(*temp, lineNumber));
				nonAlphabeticLines.AddToEnd(*",", 1);
				break;
			case -2: // Character size exceeds VERB_CHAR_SIZE size
				_charExceedsSize++;
				exceedsSizeLines.AddToEnd(*temp, IntToStr(*temp, lineNumber));
				exceedsSizeLines.AddToEnd(*",", 1);
				break;
			case -3: // Character size smaller than 2 characters
				_charSizeSmaller2++;
				sizeSmaller2CharLines.AddToEnd(*temp, IntToStr(*temp, lineNumber));
				sizeSmaller2CharLines.AddToEnd(*",", 1);
				break;
			case -4: // The verb is already exist
				_notUnique++;
				nonUniqueLines.AddToEnd(*temp, IntToStr(*temp, lineNumber));
				nonUniqueLines.AddToEnd(*",", 1);
				break;
			case 2: // Empty Lines
				_emptyLine++;
				emptyLines.AddToEnd(*temp, IntToStr(*temp, lineNumber));
				emptyLines.AddToEnd(*",", 1);
				break;
			};

			findNextLine((&file)[i]); // find the next line from file

			i += lineLenght;
			lineNumber++;
		} while ((&file)[i] != '\0');

		if (_successfull != 0)
			Log::Info(_successfull) << " verbs added successfully.\n";
		if (_nonAlphabetic != 0) {
			Log::Error(_nonAlphabetic) << " verbs contains non-alphabetic characters.\n" <<
				"These lines; " << nonAlphabeticLines.content << "\n";
		}
		if (_charExceedsSize != 0) {
			Log::Error(_charExceedsSize) << " verbs exceed " << VERB_CHAR_SIZE << " characters.\n" <<
				"These lines; " << exceedsSizeLines.content << "\n";
		}
		if (_charSizeSmaller2 != 0) {
			Log::Error(_charSizeSmaller2) << " verbs are smaller than 2 characters.\n" <<
				"These lines; " << sizeSmaller2CharLines.content << "\n";
		}
		if (_notUnique != 0) {
			Log::Error(_notUnique) << " verbs are not unique.\n" <<
				"These lines; " << nonUniqueLines.content << "\n";
		}
		if (_emptyLine != 0) {
			Log::Warning(_emptyLine) << " empty lines.\n" <<
				"These lines; " << emptyLines.content << "\n";
		}
	}

	int8 VerbHandler::AddNewVerb(const char& line) {
		// return 2   : Empty line
		// return +1  : The verb Successfully added
		// return -1  : There is some charachter which is not alphabetic
		// return -2  : Character size exceeds VERB_CHAR_SIZE size
		// return -3  : Character size smaller than 2 characters
		// return -4  : The verb is already exist

		char temp[150];
		int length = 0;
		uint8 regularity_id = 1;
		bool parenthesOpened = false;
		bool checkUniqueness = true;
		SuffixGroup exception_data(Undefined, Undefined, Undefined);

		for (int i = 0; i < sizeof(temp); i++) {
			char currentCh = ToLowerCase((&line)[i]);

			if (currentCh != 0) { // When char is in a-Z
				temp[length++] = currentCh;
			}
			else if ((&line)[i] == ' ' || (&line)[i] == '\t') // If char is blank space or new tab
				continue;
			else if ((&line)[i] == '\n' || (&line)[i] == '\0' || (&line)[i] == '#') // when new line or terminal char 
			{
				if (length > 1) { // If the temp array is not empty
					if (length > VERB_CHAR_SIZE) {
						return -2;
					}
					if (regularity_id == 1) // the verb is regular
					{
						if (checkUniqueness) { // Default value is true
							if (FindVerb(*temp, length) == 0) // Check Uniqueness
								CreateNewVerb(*temp, length, exception_data);
							else
								return -4; // The base verb is already exist
						}
						else { // If user disabled this feature
							CreateNewVerb(*temp, length, exception_data);
						}
					}
					else { // Ending of the irregular verbs
						exception_data.ed = IrregularVerb_V3;
						Verb* verb_adress = &CreateNewVerb(*temp, length, exception_data);
						CreateIrregularVerb(*verb_adress);
					}
				}
				else if (length == 1) { // Verb cannot be one char
					return -3;
				}

				break;
			}
			else if ((&line)[i] == '-')  // some verbs can contain '-'
				temp[length++] = (&line)[i];
			else if ((&line)[i] == ',') { // irregular verb input
				// Length control
				if (length > VERB_CHAR_SIZE) { // charachter size exceeds VERB_CHAR_SIZE size
					return -2;
				}
				else if (length == 0) { // When there is no word in temp
					continue;
				}
				else if (length == 1) { // there can be no verb with one char
					return -3;
				}


				// If parathes opened in the back
				if (parenthesOpened) {
					Verb* verb_adress = nullptr;


					if (regularity_id == 2) {
						// Create new verb in index buffer THEN add irregular verb to the last collection
						exception_data.ed = IrregularVerb_V2;
						verb_adress = &CreateNewVerb(*temp, length, exception_data);
					}
					else if (regularity_id == 3) {
						// Create new verb in index buffer THEN add irregular verb to the last collection
						exception_data.ed = IrregularVerb_V3;
						verb_adress = &CreateNewVerb(*temp, length, exception_data);
					}
					else if (regularity_id == 1) {
						char warning[VERB_CHAR_SIZE];
						CopyData(*warning, *temp, length);
						warning[length] = '\0';
						Log::Warning("Base verb '") << warning <<
							"' cannot start with parentheses! Please find and remove it!\n";
						return -1;
					}

					CreateIrregularVerb(*verb_adress);
					length = 0;
				}
				else { // Parentheses closed correctly
					Verb* verb_adress = nullptr;

					if (regularity_id == 1) { // the verb is V1
						// Create new verb in index buffer THEN create irregular verb collection
						if (FindVerb(*temp, length) == 0) {// If base form is unique
							exception_data.ed = IrregularVerb;
							verb_adress = &CreateNewVerb(*temp, length, exception_data);
						}
						else
							return -4; // The base verb is already exist

						exception_data.s = Undefined;
						exception_data.ing = Undefined;
					}
					else if (regularity_id == 2) {
						// Create new verb in index buffer THEN add irregular verb to the last collection
						exception_data.ed = IrregularVerb_V2;
						verb_adress = &CreateNewVerb(*temp, length, exception_data);
					}
					else if (regularity_id == 3) { // When you forgot the closing the parantheses at the V3
						exception_data.ed = IrregularVerb_V3;
						verb_adress = &CreateNewVerb(*temp, length, exception_data);
						char warning[VERB_CHAR_SIZE];
						CopyData(*warning, *temp, length);
						warning[length] = '\0';
						Log::Warning("At the end of this verb '") << warning <<
							"' ends with comma(',') please find and remove it!\n";
						break;
					}

					CreateIrregularVerb(*verb_adress);
					length = 0;
					regularity_id += 1;
				}
			}
			else if ((&line)[i] == '(') {
				parenthesOpened = true;
			}
			else if ((&line)[i] == ')') { // Add last verb 
				parenthesOpened = false;

				// Length control
				if (length > VERB_CHAR_SIZE) { // charachter size exceeds VERB_CHAR_SIZE size
					return -2;
				}
				else if (length == 0) { // When there is no word in temp
					continue;
				}
				else if (length == 1) { // there can be no verb with one char
					return -3; // One char exception
				}

				// Add verbs according to their regularity
				if (regularity_id == 2) {
					// Create new verb in index buffer THEN add irregular verb to the last collection
					exception_data.ed = IrregularVerb_V2;
					Verb* verb_adress = &CreateNewVerb(*temp, length, exception_data);
					CreateIrregularVerb(*verb_adress);
				}
				else if (regularity_id == 3) {
					// Create new verb in index buffer THEN add irregular verb to the last collection
					exception_data.ed = IrregularVerb_V3;
					Verb* verb_adress = &CreateNewVerb(*temp, length, exception_data);
					CreateIrregularVerb(*verb_adress);
					break;
				}

				length = 0;
				regularity_id += 1;
			}
			else if ((&line)[i] == '/') {
				if (DoesContain(*"I0", (&line)[i + 1])) { // I0: ing except none
					exception_data.ing = None;
					i += 2;
				}
				else if (DoesContain(*"S0", (&line)[i + 1])) { // S0 : S except none
					exception_data.s = None;
					i += 2;
				}
				else if (DoesContain(*"DL", (&line)[i + 1])) { // DL : Double last char
					exception_data.ing = Suffix_X_ing;
					exception_data.ed = Suffix_X_ed;
					i += 2;
				}
				else if (DoesContain(*"U0", (&line)[i + 1])) { // U0 : don't check uniqueness
					checkUniqueness = false;
					i += 2;
				}
				else {
					return -1; // There is some character which is not alphabetic
				}
			}
			else
				return -1; // There is some character which is not alphabetic
		}

		if (length > 0)
			return 1; // Successfly added
		else
			return 2; // Line was empty
	}


	void VerbHandler::CreateIrregularVerb(Verb& verb) {
		if (&verb == nullptr) {
			// Do nothing
		}
		else if (verb.suffixes.ed == IrregularVerb) { // Create new collection
			if (irrVerbCollection.capacity > irrVerbCollection.amount) { // If there is enough space for new collection
				// Do nothing
			}
			else { // IF space is not enough
				if (irrVerbCollection.capacity > 0) { // If current capacity bigger than 0
					int newSize = irrVerbCollection.capacity * SIZE_INCREASE_COEFFICIENT;
					// Hold old pointer
					IrrVerb* p_old_data = irrVerbCollection.verbs;
					// Create new heap space
					irrVerbCollection.verbs = new IrrVerb[newSize];
					// Update new capacity
					irrVerbCollection.capacity = newSize;
					// Copy Old data to new one
					CopyData(*irrVerbCollection.verbs, *p_old_data, irrVerbCollection.amount);
					// Free old space
					delete[] p_old_data;
				}
				else { // init a new collection
					irrVerbCollection.verbs = new IrrVerb[VERB_SIZE];
					irrVerbCollection.capacity = VERB_SIZE;
				}
			}

			// Assign verb address to base form and make others null
			irrVerbCollection.verbs[irrVerbCollection.amount].baseForm = &verb;
			irrVerbCollection.verbs[irrVerbCollection.amount].pastSimple = nullptr;
			irrVerbCollection.verbs[irrVerbCollection.amount].pastSimple_2 = nullptr;
			irrVerbCollection.verbs[irrVerbCollection.amount].pastParticiple = nullptr;
			irrVerbCollection.verbs[irrVerbCollection.amount].pastParticiple_2 = nullptr;
			// Increase irregular verb collection amount by 1
			irrVerbCollection.amount += 1;
		}
		else if (verb.suffixes.ed == IrregularVerb_V2) { // use the last collection PAST SIMPLE
			if (irrVerbCollection.verbs[irrVerbCollection.amount - 1].pastSimple == nullptr)
				irrVerbCollection.verbs[irrVerbCollection.amount - 1].pastSimple = &verb;
			else
				irrVerbCollection.verbs[irrVerbCollection.amount - 1].pastSimple_2 = &verb;
		}
		else if (verb.suffixes.ed == IrregularVerb_V3) { // use the last collection PAST PARTICIPLE
			if (irrVerbCollection.verbs[irrVerbCollection.amount - 1].pastParticiple == nullptr)
				irrVerbCollection.verbs[irrVerbCollection.amount - 1].pastParticiple = &verb;
			else
				irrVerbCollection.verbs[irrVerbCollection.amount - 1].pastParticiple_2 = &verb;
		}
	}

	Verb& VerbHandler::CreateNewVerb(const char& verb_chars, const int& str_lenght, const SuffixGroup& exception_p) {
		int row = (&verb_chars)[0] - 97; // 97 means 'a' in ASCII code
		int col;

		if ((&verb_chars)[1] == '-')
			col = 26; // 26 reserved for -
		else
			col = (&verb_chars)[1] - 97; // 97 means 'a' in ASCII code


		// Check buffer size
		if (buffer[row * VERB_COL + col].verbCapacity > buffer[row * VERB_COL + col].verbAmount) {
			// There is enough space for new verb
		}
		else { // There is no empyt slot to put new word
			if (buffer[row * VERB_COL + col].verbCapacity > 0) { // If current capacity bigger than 0
				// Calculating coefficient for new verb heap size
				int newSize = buffer[row * VERB_COL + col].verbCapacity * SIZE_INCREASE_COEFFICIENT;
				// Hold old pointer
				Verb* p_old_data = buffer[row * VERB_COL + col].verbs;
				// Create new verbs array
				buffer[row * VERB_COL + col].verbs = new Verb[newSize];
				// Update new capacity
				buffer[row * VERB_COL + col].verbCapacity = newSize;
				// Copy Old data to new array
				for (uint32 item = 0; item < buffer[row * VERB_COL + col].verbAmount; item++) {
					// Copy data
					buffer[row * VERB_COL + col].verbs[item] = p_old_data[item];
					// If the verb is irregular update the adress
					if (buffer[row * VERB_COL + col].verbs[item].suffixes.ed == IrregularVerb ||
						buffer[row * VERB_COL + col].verbs[item].suffixes.ed == IrregularVerb_V2 ||
						buffer[row * VERB_COL + col].verbs[item].suffixes.ed == IrregularVerb_V3) {
						UpdateIrrVerbAdress(buffer[row * VERB_COL + col].verbs[item], p_old_data[item]);
					}

				}

				//CopyData(*buffer[row * VERB_COL + col].verbs, *p_old_data, buffer[row * VERB_COL + col].verbAmount);
				// Free old space
				delete[] p_old_data;
			}
			else { // Initialize new heap array
				if (col == 26) { // index for '-' i.e "x-"
					buffer[row * VERB_COL + col].verbs = new Verb[VERB_SIZE_HYPHEN];
					buffer[row * VERB_COL + col].verbCapacity = VERB_SIZE_HYPHEN;
				}
				else {
					buffer[row * VERB_COL + col].verbs = new Verb[VERB_SIZE];
					buffer[row * VERB_COL + col].verbCapacity = VERB_SIZE;
				}

			}
		}

		// Copy chars to buffer verb and increase the size of verb
		CopyData(*buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].chars, verb_chars, str_lenght);
		buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].length = str_lenght;

		// Check Exceptions... -S -ING -ED
		if (exception_p.ed == IrregularVerb)
		{
			// Change regularity of the verb
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ed = IrregularVerb;

			// Check S exception
			if (exception_p.s == Undefined) {
				CheckException_S(buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount]);
			}
			else {
				buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.s = exception_p.s;
			}
			// Check ING exception
			if (exception_p.ing == Undefined) {
				CheckException_ING(buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount]);
			}
			else {
				buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ing = exception_p.ing;
			}
		}
		else if (exception_p.ed == IrregularVerb_V2) {
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ed = IrregularVerb_V2;
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ing = IrregularVerb_V2;
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.s = IrregularVerb_V2;
		}
		else if (exception_p.ed == IrregularVerb_V3) {
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ed = IrregularVerb_V3;
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ing = IrregularVerb_V3;
			buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.s = IrregularVerb_V3;
		}
		else { // When the verb is regular
			// Check ED exception
			if (exception_p.ed == Undefined) {
				CheckException_ED(buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount]);
			}
			else {
				buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ed = exception_p.ed;
			}
			// Check S exception
			if (exception_p.s == Undefined) {
				CheckException_S(buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount]);
			}
			else {
				buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.s = exception_p.s;
			}
			// Check ING exception
			if (exception_p.ing == Undefined) {
				CheckException_ING(buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount]);
			}
			else {
				buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount].suffixes.ing = exception_p.ing;
			}

		}

		// Increase last empty index by one
		buffer[row * VERB_COL + col].verbAmount += 1;

		// Return the newly added verb's adress
		return buffer[row * VERB_COL + col].verbs[buffer[row * VERB_COL + col].verbAmount - 1];
	}

#define VERB_RESULT_ARRAY_SIZE 5
	int8 VerbHandler::ParseVerb(const char& raw_chars, char& out_result, bool parse_flag) const {
		// return 2   : Empty line
		// return -1  : No verb found.
		// return +1  : The verb Successfully found.
		// return -2  : There is some characters which is not alphabetic
		// return -3  : First character cannot be hyphen
		// return -4  : Character size less than 2 characters
		// return -5  : Character size exceeds VERB_CHAR_SIZE size

		char verb_chars[150];
		uint8 lenght = 0;

		// Make all chars lowercase and get rid of spaces
		for (int i = 0; (&raw_chars)[i] != '\0'; i++) {
			char currentCh = ToLowerCase((&raw_chars)[i]);

			if (currentCh != 0) // If the char is alphabetic
				verb_chars[lenght++] = currentCh;
			else if ((&raw_chars)[i] == ' ' || (&raw_chars)[i] == '\t')
				continue;
			else if ((&raw_chars)[i] == '-') { // If the charachter is hyphen
				if (i == 0)
					return -3; // First character cannot be hyphen
				else
					verb_chars[lenght++] = (&raw_chars)[i]; // Add hy
			}
			else
				return -2; // There is some character which is not alphabetic
		}

		// Size control
		if (lenght < 2)
			return -4; // Character size smaller than 2 characters
		else if (lenght > VERB_CHAR_SIZE)
			return -5; // Character size exceeds VERB_CHAR_SIZE size

		// Create verb holder and assing nullptr
		Verb* foundVerbs[VERB_RESULT_ARRAY_SIZE];
		for (uint8 i = 0; i < VERB_RESULT_ARRAY_SIZE; i++)
			foundVerbs[i] = nullptr;

		// Search if the verb in native form
		int8 resultVerbsAmount = FindVerb(*verb_chars, lenght, true, foundVerbs);
		if (resultVerbsAmount > 0) {
			// Turning verb data into str
			VerbsToStr(foundVerbs, VERB_RESULT_ARRAY_SIZE, out_result);
			return 1;
		}

		if (parse_flag) {
			int lastIndex = 0;

			// Search if the verb has -ed
			{
				lastIndex += CopyStr((&out_result)[lastIndex], *"\x1b[95m[Infectional Verb (-ed)]: \x1b[0m");
				bool is_ed = true;
				switch (ED_Parser(*verb_chars, lenght, foundVerbs))
				{
				case -1:
					lastIndex = 0;
					is_ed = false;
					break;
				case None:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + ed\n");
					break;
				case Suffix_0y_ied:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *"(-y) + ied\n");
					break;
				case Suffix_d:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + d\n");
					break;
				case Suffix_X_ed:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + (");
					(&out_result)[lastIndex++] = verb_chars[lenght - 3];
					lastIndex += CopyStr((&out_result)[lastIndex], *")ed\n");
					break;
				default:
					Log::Error("ED parser returns a value that doesn't exist in switch.");
					return -1;
				}

				if (is_ed) { // If we found the verb in -ed tags
					// Turning verb data into str
					VerbsToStr(foundVerbs, VERB_RESULT_ARRAY_SIZE, (&out_result)[lastIndex]);
					return 1;
				}
			}

			// Search if the verb has -ing
			{
				lastIndex += CopyStr((&out_result)[lastIndex], *"\x1b[95m[Infectional Verb (-ing)]: \x1b[0m");
				bool is_ing = true;

				// None, EndsWith_w_x_y, EndsWith_Ce, EndsWith_ie, EndsWith_XVC, DoubleLastChar 

				switch (ING_Parser(*verb_chars, lenght, foundVerbs))
				{
				case -1:
					lastIndex = 0;
					is_ing = false;
					break;
				case None:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + ing\n");
					break;
				case Suffix_0e_ing:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *"(-e) + ing\n");
					break;
				case Suffix_0ie_ying:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *"(-ie) + ying\n");
					break;
				case Suffix_X_ing:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + (");
					(&out_result)[lastIndex++] = verb_chars[lenght - 4];
					lastIndex += CopyStr((&out_result)[lastIndex], *")ing\n");
					break;
				default:
					Log::Error("ING parser returns a value that doesn't exist in switch.");
					return -1;
				}

				if (is_ing) { // If we found the verb in -ed tags
					// Turning verb data into str
					VerbsToStr(foundVerbs, VERB_RESULT_ARRAY_SIZE, (&out_result)[lastIndex]);
					return 1;
				}
			}

			// Search if the verb has -s
			{
				lastIndex += CopyStr((&out_result)[lastIndex], *"\x1b[95m[Infectional Verb (-s)]: \x1b[0m");
				bool is_es = true;

				switch (S_Parser(*verb_chars, lenght, foundVerbs))
				{
				case -1:
					lastIndex = 0;
					is_es = false;
					break;
				case None:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + s\n");
					break;
				case Suffix_es:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + es\n");
					break;
				case Suffix_ses:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + ses\n");
					break;
				case Suffix_zes:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *" + zes\n");
					break;
				case Suffix_0y_ies:
					lastIndex += CopyStr((&out_result)[lastIndex], *(foundVerbs[0]->chars), foundVerbs[0]->length);
					lastIndex += CopyStr((&out_result)[lastIndex], *"(-y) + ies\n");
					break;
				default:
					Log::Error("S parser returns a value that doesn't exist in switch");
					return -1;
				}

				if (is_es) { // If we found the verb in -ed tags
					// Turning verb data into str
					VerbsToStr(foundVerbs, VERB_RESULT_ARRAY_SIZE, (&out_result)[lastIndex]);
					return 1;
				}
			}

		}

		return -1;
	}

#pragma region Suffix Parser
	int16 VerbHandler::ED_Parser(const char& verb, const uint8& lenght, Verb** out_verbs) const {
		// return  -1: verb couldn't find
		// return  X: verb found and with exception

		// None, EndsWith_w_x_y, EndsWith_Cy, EndsWith_e, EndsWith_XVC, DoubleLastChar
		// -ed,		-ed,		-ied		, -d		, -X + ED		,-X + ED 	  

		SuffixGroup exceptions(Undefined, Undefined, Undefined);

		int8 result = 0;

		if ((&verb)[lenght - 1] == 'd') { // when the char is -d
			// If the input ends with d
			exceptions.ed = Suffix_d; // EndsWith_e
			result = SearchVerb(verb, lenght - 1, out_verbs, exceptions);
			if (result > 0) // If verbs ends with -e
				return Suffix_d;

			// If the input ends with -ed
			if ((&verb)[lenght - 2] == 'e') { // when the last 2 chars is -ed
				exceptions.ed = None;
				result = SearchVerb(verb, lenght - 2, out_verbs, exceptions);

				if (result > 0)  // If verbs ends with None
					return None;

				if (lenght > 2 && (&verb)[lenght - 3] == 'i') {
					char tempVerb[VERB_CHAR_SIZE]; // Creating a temporary space for verb 
					CopyData(*tempVerb, verb, lenght - 3); // copying charachters excluding last 3
					tempVerb[lenght - 3] = 'y'; // adding y to the last char of the verb 

					exceptions.ed = Suffix_0y_ied; //EndsWith_Cy
					result = SearchVerb(*tempVerb, lenght - 2, out_verbs, exceptions);

					if (result > 0)  // If verbs ends with  Cy
						return Suffix_0y_ied;
				}
				else if (lenght > 5 && !IsVowel((&verb)[lenght - 3])) {
					// verb lenght has to be bigger than 5 the 3rd to last has to be consonant

					if ((&verb)[lenght - 3] == (&verb)[lenght - 4]) { // If the 3rd to last and 4th chars are same
						exceptions.ed = Suffix_X_ed;
						result = SearchVerb(verb, lenght - 3, out_verbs, exceptions);
						if (result > 0)  // If verbs ends with consonants + Vovel + Consonant
							return Suffix_X_ed;
					}
				}
			}
		}

		return -1;
	}

	int16 VerbHandler::ING_Parser(const char& verb, const uint8& lenght, Verb** out_verbs) const {
		// None, EndsWith_w_x_y, EndsWith_Ce, EndsWith_ie, EndsWith_XVC, DoubleLastChar 
		// -ing,	-ing,		 -(-e)ing,		-ying,			-xxing,		-xxing

		SuffixGroup exceptions(Undefined, Undefined, Undefined);
		int8 result = 0;

		if (lenght > 4) { // the lengh has to be min 5 chars
			// If the input str ends with -ing
			if ((&verb)[lenght - 3] == 'i' && (&verb)[lenght - 2] == 'n' && (&verb)[lenght - 1] == 'g') {
				char tempVerb[VERB_CHAR_SIZE]; // Creating a temporary space for verb 

				exceptions.ing = None;
				result = SearchVerb(verb, lenght - 3, out_verbs, exceptions);
				if (result > 0) // If verbs ends with none
					return None;

				if (!IsVowel((&verb)[lenght - 4])) { // If the 4th to last char is Consonant

					{ // If the original verb ends with -e
						CopyData(*tempVerb, verb, lenght - 3); // copying charachters excluding last 3
						tempVerb[lenght - 3] = 'e'; // adding e to the last char of the verb 

						exceptions.ing = Suffix_0e_ing; // EndsWith_Ce
						result = SearchVerb(*tempVerb, lenght - 2, out_verbs, exceptions);

						if (result > 0)  // If verbs ends with Cy
							return Suffix_0e_ing;
					}

					if (lenght > 6 && (&verb)[lenght - 4] == (&verb)[lenght - 5]) { // If the input str ends with double last char
						exceptions.ing = Suffix_X_ing;
						result = SearchVerb(verb, lenght - 4, out_verbs, exceptions);
						if (result > 0)  // If verbs ends with consonants + Vovel + Consonant
							return Suffix_X_ing;
					}
				}

				if ((&verb)[lenght - 4] == 'y') { // If the input text ends with -ying
					CopyData(*tempVerb, verb, lenght - 4); // copying charachters excluding last 4
					CopyStr(tempVerb[lenght - 4], *"ie", 2);// copying ie to the end

					exceptions.ing = Suffix_0ie_ying; // EndsWith_ie
					result = SearchVerb(*tempVerb, lenght - 2, out_verbs, exceptions);

					if (result > 0)  // If verbs ends with  Cy
						return Suffix_0ie_ying;
				}
			}
		}

		return -1;
	}

	int16 VerbHandler::S_Parser(const char& verb, const uint8& lenght, Verb** out_verbs) const {
		// None, EndsWith_Cy, EndsWith_ss, EndsWith_zz, EndsWith_ch, EndsWith_sh, EndsWith_s, EndsWith_z, EndsWith_x, EndsWith_o
		// -s		-(-y)ies	-es			   -es			-es			-es			 -ses		 -zes	,	 -es    ,	-es

		SuffixGroup exceptions(Undefined, Undefined, Undefined);
		int8 result = 0;

		if (lenght > 2) { // the input text has to be at least 3 chars
			if ((&verb)[lenght - 1] == 's') { // input str ends with -s
				{
					exceptions.s = None;
					result = SearchVerb(verb, lenght - 1, out_verbs, exceptions);

					if (result > 0)
						return None;
				}
				if ((&verb)[lenght - 2] == 'e') { // input str ends with -es
					{
						exceptions.s = Suffix_es;
						result = SearchVerb(verb, lenght - 2, out_verbs, exceptions);

						if (result > 0)
							return Suffix_es;
					}
					if ((&verb)[lenght - 3] == 'i') { // input str ends with -ies
						char tempVerb[VERB_CHAR_SIZE]; // Creating a temporary space for verb
						CopyData(*tempVerb, verb, lenght - 3); // copying charachters excluding last 3
						tempVerb[lenght - 3] = 'y'; // adding e to the last char of the verb 
						exceptions.s = Suffix_0y_ies; // setting the suffix type

						result = SearchVerb(*tempVerb, lenght - 2, out_verbs, exceptions);
						if (result > 0)
							return Suffix_0y_ies;
					}
					if (lenght > 4) {
						if ((&verb)[lenght - 3] == 's' && (&verb)[lenght - 4] == 's') { // input str ends with -sses
							exceptions.s = Suffix_ses;
							result = SearchVerb(verb, lenght - 3, out_verbs, exceptions);
							if (result > 0)
								return Suffix_ses;
						}
						else if ((&verb)[lenght - 3] == 'z' && (&verb)[lenght - 4] == 'z') { // input str ends with -zzes
							exceptions.s = Suffix_zes;
							result = SearchVerb(verb, lenght - 3, out_verbs, exceptions);
							if (result > 0)
								return Suffix_zes;
						}
					}
				}
			}
		}

		return -1;
	}

#pragma endregion

	int8 VerbHandler::SearchVerb(const char& verb_chars, const int& length, Verb**& out_verbs, SuffixGroup& exception_p) const {
		// return -1 : no identifer
		// return 0  : no verb found
		// return 1  : 1 verb found
		// return 2  : 2 verb found
		// return 3  : 3 verb found.
		// return VERB_RESULT_ARRAY_SIZE+1 : founded verbs exceeds VERB_RESULT_ARRAY_SIZE size

		int8 lastId = 0;

		if (length < 2) {
			return 0;
		}

		int row = (&verb_chars)[0] - 97; // 97 means 'a' in ASCII code
		int col;
		if ((&verb_chars)[1] == '-')
			col = 26; // 26 reserved for -
		else
			col = (&verb_chars)[1] - 97; // 97 means 'a' in ASCII code


		bool (*ed_type)(const SuffixGroup&, Verb&) = [](const SuffixGroup& ee, Verb& verb) {
			return ee.ed == verb.suffixes.ed;
		};

		bool (*s_type)(const SuffixGroup&, Verb&) = [](const SuffixGroup& ee, Verb& verb) {
			return ee.s == verb.suffixes.s;
		};

		bool (*ing_type)(const SuffixGroup&, Verb&) = [](const SuffixGroup& ee, Verb& verb) {
			return ee.ing == verb.suffixes.ing;
		};

		bool (*IsSameType)(const SuffixGroup&, Verb&);

		if (exception_p.ed != Undefined) {
			IsSameType = ed_type;
		}
		else if (exception_p.s != Undefined) {
			IsSameType = s_type;
		}
		else if (exception_p.ing != Undefined) {
			IsSameType = ing_type;
		}
		else {
			return -1;
		}


		for (int i = 0; i < buffer[row * VERB_COL + col].verbAmount; i++) { // Get all verbs from buffer
			//If they have same lenght
			if (buffer[row * VERB_COL + col].verbs[i].length == length) {
				// If they have same type
				if (IsSameType(exception_p, buffer[row * VERB_COL + col].verbs[i])) {
					// If they have same content
					if (IsSameStr(*buffer[row * VERB_COL + col].verbs[i].chars, verb_chars, length)) {
						if (lastId < VERB_RESULT_ARRAY_SIZE)
						{
							out_verbs[lastId] = &buffer[row * VERB_COL + col].verbs[i];
							lastId++;
						}
						else {
							return VERB_RESULT_ARRAY_SIZE;
						}
					}
				}
			}
		}

		return lastId;
	}

	int8 VerbHandler::FindVerb(const char& verb_chars, const int& length, bool hold_verb, Verb** out_verbs) const {
		// return -1 : verb is smaller than 2 chars
		// return 0  : no verb found
		// return 1  : 1 verb found
		// return 2  : 2 verb found
		// return 3  : 3 verb found
		// return VERB_RESULT_ARRAY_SIZE+1 : founded verbs exceeds VERB_RESULT_ARRAY_SIZE size

		int8 lastId = 0;

		if (length < 2) // Verb cannot be one character
			return -1;

		int row = (&verb_chars)[0] - 97; // 97 means 'a' in ASCII code

		int col;

		if ((&verb_chars)[1] == '-')
			col = 26; // 26 reserved for -
		else
			col = (&verb_chars)[1] - 97; // 97 means 'a' in ASCII code

		for (int i = 0; i < buffer[row * VERB_COL + col].verbAmount; i++) { // Get all verbs from buffer
			if (buffer[row * VERB_COL + col].verbs[i].length == length) {
				if (IsSameStr(*buffer[row * VERB_COL + col].verbs[i].chars, verb_chars, length)) {
					if (hold_verb) {
						if (lastId < VERB_RESULT_ARRAY_SIZE)
						{
							(out_verbs)[lastId] = &buffer[row * VERB_COL + col].verbs[i];
							lastId++;
						}
						else {
							return VERB_RESULT_ARRAY_SIZE;
						}
					}
					else {
						lastId++;
						return lastId;
					}
				}
			}
		}

		return lastId;
	}

#undef VERB_RESULT_ARRAY_SIZE

	uint32 VerbHandler::VerbToStr(const Verb& verb, char& out_string) const {
		if (&verb == nullptr) // If the object is empty return 0
			return 0;

		uint32 lastIndex = 0;
		CopyData(out_string, *verb.chars, verb.length);
		lastIndex += verb.length;

		lastIndex += CopyStr((&out_string)[lastIndex], *"\t[Sfx_ED]: ");
		lastIndex += ExceptionToStr(0, verb.suffixes.ed, (&out_string)[lastIndex]);

		lastIndex += CopyStr((&out_string)[lastIndex], *", [Sfx_S]: ");
		lastIndex += ExceptionToStr(1, verb.suffixes.s, (&out_string)[lastIndex]);

		lastIndex += CopyStr((&out_string)[lastIndex], *", [Sfx_ING]: ");
		lastIndex += ExceptionToStr(2, verb.suffixes.ing, (&out_string)[lastIndex]);
		return lastIndex;
	}

	uint32 VerbHandler::VerbsToStr(Verb* const* verbs, uint16 verb_size, char& out_string) const {
		if (verbs[0] == nullptr) // If the object is empty return 0
			return 0;

		uint32 lastIndex = 0;

		for (int i = 0; i < verb_size; i++) {
			if (verbs[i] != nullptr) {
				lastIndex += IntToStr((&out_string)[lastIndex], i + 1);
				lastIndex += CopyStr((&out_string)[lastIndex], *". ");
				lastIndex += VerbToStr(*verbs[i], (&out_string)[lastIndex]);
				(&out_string)[lastIndex++] = '\n';
			}
			else {
				break;
			}
		}
		(&out_string)[lastIndex] = '\0';

		return lastIndex;
	}

	void VerbHandler::GetVerbsWithIndex(const char& index_couple, char& out_string) {
		int row = (&index_couple)[0] - 97; // 97 means 'a' in ASCII code
		int col;

		if ((&index_couple)[1] == '-')
			col = 26; // 26 reserved for -
		else
			col = (&index_couple)[1] - 97; // 97 means 'a' in ASCII code


		uint32 lastIndex = 0;
		for (int i = 0; i < buffer[row * VERB_COL + col].verbAmount; i++) {
			lastIndex += VerbToStr(buffer[row * VERB_COL + col].verbs[i], (&out_string)[lastIndex]);
		}

		(&out_string)[lastIndex] = '\0';
	}

	int8 VerbHandler::UpdateIrrVerbAdress(Verb& new_address, const Verb& old_adress) {

		for (uint16 i = 0; i < irrVerbCollection.amount; i++) {
			if (new_address.suffixes.ed == IrregularVerb) {
				if (irrVerbCollection.verbs[i].baseForm == &old_adress) {
					irrVerbCollection.verbs[i].baseForm = &new_address;
					return 1;
				}
			}
			else if (new_address.suffixes.ed == IrregularVerb_V2) {
				if (irrVerbCollection.verbs[i].pastSimple == &old_adress) {
					irrVerbCollection.verbs[i].pastSimple = &new_address;
					return 1;
				}
				else if (irrVerbCollection.verbs[i].pastSimple_2 == &old_adress) {
					irrVerbCollection.verbs[i].pastSimple_2 = &new_address;
					return 1;
				}
			}
			else if (new_address.suffixes.ed == IrregularVerb_V3) {
				if (irrVerbCollection.verbs[i].pastParticiple == &old_adress) {
					irrVerbCollection.verbs[i].pastParticiple = &new_address;
					return 1;
				}
				else if (irrVerbCollection.verbs[i].pastParticiple_2 == &old_adress) {
					irrVerbCollection.verbs[i].pastParticiple_2 = &new_address;
					return 1;
				}
			}

		}

		return -1; // Verb couln't found in irregular verbs
	}

	uint16 VerbHandler::GetAllIrregularVerbs(char& out_string) const {
		int lastCharIndex = 0;

		for (int i = 0; i < irrVerbCollection.amount; i++) {
			// Pase form copy
			lastCharIndex += CopyStr((&out_string)[lastCharIndex], *"[Base]:");
			CopyData((&out_string)[lastCharIndex], *irrVerbCollection.verbs[i].baseForm->chars,
				irrVerbCollection.verbs[i].baseForm->length);
			lastCharIndex += irrVerbCollection.verbs[i].baseForm->length;

			// Past simple forms copy
			if (irrVerbCollection.verbs[i].pastSimple != nullptr) {
				lastCharIndex += CopyStr((&out_string)[lastCharIndex], *"\t[PastSimple]:");

				CopyData((&out_string)[lastCharIndex], *irrVerbCollection.verbs[i].pastSimple->chars,
					irrVerbCollection.verbs[i].pastSimple->length);
				lastCharIndex += irrVerbCollection.verbs[i].pastSimple->length;

				if (irrVerbCollection.verbs[i].pastSimple_2 != nullptr) {
					lastCharIndex += CopyStr((&out_string)[lastCharIndex], *", ");
					CopyData((&out_string)[lastCharIndex], *irrVerbCollection.verbs[i].pastSimple_2->chars,
						irrVerbCollection.verbs[i].pastSimple_2->length);
					lastCharIndex += irrVerbCollection.verbs[i].pastSimple_2->length;
				}
			}

			// Past Participle forms copy
			if (irrVerbCollection.verbs[i].pastParticiple != nullptr) {
				lastCharIndex += CopyStr((&out_string)[lastCharIndex], *"\t[PastParticiple]:");

				CopyData((&out_string)[lastCharIndex], *irrVerbCollection.verbs[i].pastParticiple->chars,
					irrVerbCollection.verbs[i].pastParticiple->length);
				lastCharIndex += irrVerbCollection.verbs[i].pastParticiple->length;

				if (irrVerbCollection.verbs[i].pastParticiple_2 != nullptr) {
					lastCharIndex += CopyStr((&out_string)[lastCharIndex], *", ");
					CopyData((&out_string)[lastCharIndex], *irrVerbCollection.verbs[i].pastParticiple_2->chars,
						irrVerbCollection.verbs[i].pastParticiple_2->length);
					lastCharIndex += irrVerbCollection.verbs[i].pastParticiple_2->length;
				}
			}

			(&out_string)[lastCharIndex++] = '\n';
		}

		(&out_string)[--lastCharIndex] = '\0';
		return irrVerbCollection.amount;
	}


#pragma region Exception Controllers

	void VerbHandler::CheckException_S(Verb& verb) const {
		// None, EndsWith_Cy, EndsWith_ss, EndsWith_zz, EndsWith_ch, EndsWith_sh, EndsWith_s, EndsWith_z, EndsWith_x, EndsWith_o

		// Check if second to last character is consonant and last char is y
		if (!IsVowel(verb.chars[verb.length - 2]) && verb.chars[verb.length - 1] == 'y') {
			verb.suffixes.s = Suffix_0y_ies; // EndsWith_Cy
			return;
		}
		else if (verb.chars[verb.length - 1] == 's') { // When last character is s
			if (verb.chars[verb.length - 2] == 's') // when verb ends with ss
				verb.suffixes.s = Suffix_es; // EndsWith_ss
			else
				verb.suffixes.s = Suffix_ses; // EndsWith_s
			return;
		}
		else if (verb.chars[verb.length - 1] == 'z') { // When last character is z
			if (verb.chars[verb.length - 2] == 'z') // when verb ends with zz
				verb.suffixes.s = Suffix_es; // EndsWith_zz
			else
				verb.suffixes.s = Suffix_zes; // EndsWith_z
			return;
		}
		else if (verb.chars[verb.length - 1] == 'h') { // When last character is h
			if (verb.chars[verb.length - 2] == 's') {// when verb ends with sh
				verb.suffixes.s = Suffix_es; // EndsWith_sh
				return;
			}
			else if (verb.chars[verb.length - 2] == 'c') { // when verb ends with ch
				verb.suffixes.s = Suffix_es; // EndsWith_ch
				return;
			}
		}
		else if (verb.chars[verb.length - 1] == 'x') { // When last character x
			verb.suffixes.s = Suffix_es; // EndsWith_x
			return;
		}
		else if (verb.chars[verb.length - 1] == 'o') { // When last character o
			verb.suffixes.s = Suffix_es; // EndsWith_o
			return;
		}

		verb.suffixes.s = None;

	}

	void VerbHandler::CheckException_ING(Verb& verb) const {
		// None, EndsWith_w_x_y, EndsWith_Ce, EndsWith_ie,  EndsWith_XVC, DoubleLastChar 
		// -ing,	-ing,		 -(-e)ing,		-(-ie)ying,		-xing,		-xing

		if (verb.chars[verb.length - 1] == 'w' || verb.chars[verb.length - 1] == 'x' || verb.chars[verb.length - 1] == 'y') {
			verb.suffixes.ing = None; // EndsWith_w_x_y
			return;
		}
		else if (verb.chars[verb.length - 1] == 'e') {
			if (verb.length > 2 && !IsVowel(verb.chars[verb.length - 2])) { // Verb ends with consonant + vowel
				verb.suffixes.ing = Suffix_0e_ing; // EndsWith_Ce
				return;
			}
			else if (verb.chars[verb.length - 2] == 'i') { // Verb ends with ie
				verb.suffixes.ing = Suffix_0ie_ying; // EndsWith_ie
				return;
			}
		}
		else if (!IsVowel(verb.chars[verb.length - 1])) {
			if (IsVowel(verb.chars[verb.length - 2])) {// Ends with Vowel + Constant
				if (verb.length > 2) { // Length control
					bool isAllConsonant = true;
					for (int i = 0; i < verb.length - 2; i++) {
						if (IsVowel(verb.chars[verb.length - 3 - i])) {
							isAllConsonant = false;
							break;
						}
					}
					if (isAllConsonant && verb.length > 2) { // Verb contains one vowel and ends with (Vowel + Constant)
						verb.suffixes.ing = Suffix_X_ing; // EndsWith_XVC
						return;
					}
				}
			}
		}

		verb.suffixes.ing = None;
	}

	void VerbHandler::CheckException_ED(Verb& verb) const {
		// None, EndsWith_w_x_y, EndsWith_Cy, EndsWith_e, EndsWith_XVC, DoubleLastChar
		// -ed,		-ed,		-ied		, -d		, -X + ED		,-X + ED 	  

		if (verb.chars[verb.length - 1] == 'w' || verb.chars[verb.length - 1] == 'x') {
			verb.suffixes.ed = None; // EndsWith_w_x_y
			return;
		}
		else if (verb.chars[verb.length - 1] == 'y') {
			if (!IsVowel(verb.chars[verb.length - 2])) // Consonant + y
				verb.suffixes.ed = Suffix_0y_ied; //  EndsWith_Cy
			else
				verb.suffixes.ed = None; // EndsWith_w_x_y
			return;
		}
		else if (verb.chars[verb.length - 1] == 'e') { // Verb ends with e
			verb.suffixes.ed = Suffix_d; // EndsWith_e
			return;
		}
		else if (!IsVowel(verb.chars[verb.length - 1])) {
			if (IsVowel(verb.chars[verb.length - 2])) {// Ends with Vowel + Constant
				if (verb.length < 3) // Length control
					return;

				bool isAllConsonant = true;
				for (int i = 0; i < verb.length - 2; i++) { // check to first if contains other vowel char 
					if (IsVowel(verb.chars[verb.length - 3 - i])) {
						isAllConsonant = false;
						break;
					}
				}
				if (isAllConsonant) {
					verb.suffixes.ed = Suffix_X_ed; // EndsWith_XVC
					return;
				}
			}
		}

		verb.suffixes.ed = None;
	}

#pragma endregion

	uint32 VerbHandler::ExceptionToStr(const int8 type, const Suffix_t& ex_type, char& out_str) const { // 0: ED, 1: S, 2:ING

		switch (type)
		{
		case 0: // Ed type
			switch (ex_type)
			{
			case None:
				return CopyStr(out_str, *"-ed");
			case IrregularVerb:
				return CopyStr(out_str, *"Irr verb V1");
			case IrregularVerb_V2:
				return CopyStr(out_str, *"Irr verb V2");
			case IrregularVerb_V3:
				return CopyStr(out_str, *"Irr verb V3");
			case Suffix_d:
				return CopyStr(out_str, *"-d");
			case Suffix_0y_ied:
				return CopyStr(out_str, *"(-y) + ied");
			case Suffix_X_ed:
				return CopyStr(out_str, *"(X) + ed");
			default:
				return CopyStr(out_str, *"Incompatible!");
			}
			break;

		case 1: // S type
			switch (ex_type)
			{
			case None:
				return CopyStr(out_str, *"-s");
			case Suffix_es:
				return CopyStr(out_str, *"-es");
			case Suffix_zes:
				return CopyStr(out_str, *"-zes");
			case Suffix_ses:
				return CopyStr(out_str, *"-ses");
			case Suffix_0y_ies:
				return CopyStr(out_str, *"(-y) + ies");
			default:
				return CopyStr(out_str, *"Incompatible!");
			}
			break;

		case 2: // Ing type
			switch (ex_type)
			{
			case None:
				return CopyStr(out_str, *"-ing");
			case Suffix_0e_ing:
				return CopyStr(out_str, *"(-e) + ing");
			case Suffix_0ie_ying:
				return CopyStr(out_str, *"(-ie) + ying");
			case Suffix_X_ing:
				return CopyStr(out_str, *"(X) + ing");;
			default:
				return CopyStr(out_str, *"Incompatible!");
			}
			break;

		default:
			return CopyStr(out_str, *"Incorrect Type!");
		}

		return 0;

		/* Old types
		switch (ex_type)
		{
		case verb::None:
			return CopyStr(out_str, *VERB_EX_NONE);
		case verb::IrregularVerb:
			return CopyStr(out_str, *VERB_EX_IRREGULAR_V1);
		case verb::IrregularVerb_V2:
			return CopyStr(out_str, *VERB_EX_IRREGULAR_V2);
		case verb::IrregularVerb_V3:
			return CopyStr(out_str, *VERB_EX_IRREGULAR_V3);
		case verb::DoubleLastChar:
			return CopyStr(out_str, *VERB_EX_DOUBLE_LAST);
		case verb::EndsWith_e:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_E);
		case verb::EndsWith_Cy:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_CY);
		case verb::EndsWith_z:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_Z);
		case verb::EndsWith_s:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_S);
		case verb::EndsWith_x:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_X);
		case verb::EndsWith_o:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_O);
		case verb::EndsWith_ss:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_SS);
		case verb::EndsWith_zz:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_ZZ);
		case verb::EndsWith_ch:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_CH);
		case verb::EndsWith_sh:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_SH);
		case verb::EndsWith_w_x_y:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_WXY);
		case verb::EndsWith_Ce:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_CE);
		case verb::EndsWith_ie:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_IE);
		case verb::EndsWith_XVC:
			return CopyStr(out_str, *VERB_EX_ENDSWITH_XVC);
		default:
			return CopyStr(out_str, *"Undefined");
		}*/
	}

}