// Standart libs
#include <thread>

// User defined libs
#include "main_handler.hpp"
#include "log.hpp"
#include "misc/util.hpp"

namespace handle {
	void MainHandler::Read(const WordType type, const char* data, const uint64& length, bool print_suc) {
		switch (type)
		{
		case Noun:
			noun.MultipleAdder(data, length);
			return;
		case Verb:
			verb.MultipleVerbAdder(data, length);
			return;
		case Pronoun:
			pronoun.MultipleAdder(data, length, "pronouns");
			return;
		case Adverb:
			adv.MultipleAdder(data, length, "adverbs", print_suc);
			return;
		case Adjective:
			adj.MultipleAdder(data, length, "adjectives", print_suc);
			return;
		case Preposition:
			prepos.MultipleAdder(data, length, "prepositions");
			return;
		case Conjunction:
			conj.MultipleAdder(data, length, "conjunctions");
			return;
		case Interjection:
			interj.MultipleAdder(data, length, "interjections");
			return;
		default:
			return;
		}
	}

	void MainHandler::Delete(const WordType type) {
		switch (type)
		{
		case Noun:
			noun.DeleteAll();
			return;
		case Verb:
			verb.DeleteAll();
			return;
		case Pronoun:
			pronoun.FreeAll();
			return;
		case Adverb:
			adv.FreeAll();
			return;
		case Adjective:
			adj.FreeAll();
			return;
		case Preposition:
			prepos.FreeAll();
			return;
		case Conjunction:
			conj.FreeAll();
			return;
		case Interjection:
			interj.FreeAll();
			return;
		default:
			return;
		}
	}

	void MainHandler::ParseWithType(const WordType type, const String& str) { // Parse with known type
		int32 result;
		String out_str;

		switch (type)
		{
		case Noun:
			result = noun.ParseNoun(str, out_str, true);
			PrintResult(Noun, result, out_str);
			return;
		case Verb:
			result = verb.ParseVerb(str, out_str, true);
			PrintResult(Verb, result, out_str);
			return;
		case Pronoun:
			result = pronoun.ParseWord(str, out_str, true);
			PrintResult(Pronoun, result, out_str);
			return;
		case Adverb:
			result = adv.ParseWord(str, out_str, true);
			PrintResult(Adverb, result, out_str);
			return;
		case Adjective:
			result = adj.ParseWord(str, out_str, true);
			PrintResult(Adjective, result, out_str);
			return;
		case Preposition:
			result = prepos.ParseWord(str, out_str, true);
			PrintResult(Preposition, result, out_str);
			return;
		case Conjunction:
			result = conj.ParseWord(str, out_str, true);
			PrintResult(Conjunction, result, out_str);
			return;
		case Interjection:
			result = interj.ParseWord(str, out_str, true);
			PrintResult(Interjection, result, out_str);
			return;
		default:
			return;
		}
	}

	void MainHandler::ParseMultithread(const String& raw_word, const bool print_result, std::vector<Word>* words) { // Multithread parser
		util::Array<int32, 8> result;
		String outStrs[result.GetSize()];

		// Start Threads
		std::thread noun_thread(&MainHandler::ParseNoun, this, &raw_word, &result[0], &outStrs[0], print_result);
		std::thread verb_thread(&MainHandler::ParseVerb, this, &raw_word, &result[1], &outStrs[1], print_result);
		std::thread pronoun_thread(&MainHandler::ParsePronoun, this, &raw_word, &result[2], &outStrs[2], print_result);
		std::thread adv_thread(&MainHandler::ParseAdv, this, &raw_word, &result[3], &outStrs[3], print_result);
		std::thread adj_thread(&MainHandler::ParseAdj, this, &raw_word, &result[4], &outStrs[4], print_result);
		std::thread prepos_thred(&MainHandler::ParsePrepos, this, &raw_word, &result[5], &outStrs[5], print_result);
		std::thread conj_thread(&MainHandler::ParseConj, this, &raw_word, &result[6], &outStrs[6], print_result);
		std::thread interj_thread(&MainHandler::ParseInterj, this, &raw_word, &result[7], &outStrs[7], print_result);

		// Wait for completion of threads
		noun_thread.join();
		verb_thread.join();
		pronoun_thread.join();
		adv_thread.join();
		adj_thread.join();
		prepos_thred.join();
		conj_thread.join();
		interj_thread.join();

		// Printing Part
		if (print_result) {
			int found = 0;
			for (int32 i = 0; i < 8; i++) { // Print results
				if (result[i] == 1) {
					found++;
					SetColor(14);
					std::cout << "\n[RESULT " << found << "]: \n";
					SetColor(7);
					PrintResult((WordType)(i + 1), result[i], outStrs[i]);
				}
			}
			if (!found) {
				Log::Warning("No result has found.\n");
			}
		}
		else { // Fill the words type
			bool found = false;
			for (int32 i = 0; i < 8; i++) { // Print results
				if (result[i] == 1) {
					words->back().type.push_back((WordType)(i + 1));
					found = true;
				}
			}
			if (!found) {
				words->back().type.push_back(Undefined);
			}
		}

	}

	void MainHandler::ParseSentence(const String& str) {
		String out_str;

		std::vector<Word> words;
		// Tokenize input
		tokenize.ParseString(str);
		// Parse each sentence 
		for (uint32 i = 0; tokenize.sentences[i].Tokens() != nullptr; i++) {
			words.reserve(tokenize.sentences[i].Amount());
			// Parse each token
			for (uint32 j = 0; j < tokenize.sentences[i].Amount(); j++) {
				// Copy adress of word to words
				words.push_back(Word());
				words.back().data = &tokenize.sentences[i][j];
				// Parse token
				ParseMultithread(tokenize.sentences[i][j], false, &words);
			}
			// Words to string
			out_str += "\n\x1b[38;5;198m[SENTENCE ";
			util::IntToStr(out_str, i + 1);
			out_str += "]: \n\x1b[0m";
			WordsToStr(words, out_str);

			// Reduce parse

			// Delete words in sentence
			words.clear();
		}

		// Print result
		std::cout << out_str.EndString().Chars();
		// Free tokens
		tokenize.FreeAll();
	}

	void MainHandler::ParseNoun(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = noun.ParseNoun(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParseVerb(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = verb.ParseVerb(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParsePronoun(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = pronoun.ParseWord(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParseAdv(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = adv.ParseWord(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParseAdj(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = adj.ParseWord(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParsePrepos(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = prepos.ParseWord(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParseConj(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = conj.ParseWord(*raw_word, *out_string, write_result);
	}
	void MainHandler::ParseInterj(const String* raw_word, int32* result, String* out_string, const bool write_result) {
		*result = interj.ParseWord(*raw_word, *out_string, write_result);
	}

	void MainHandler::WordsToStr(std::vector<Word>& words, String& out_str) {
		auto printWordType = [&](WordType type) {
			switch (type)
			{
			case handle::Undefined:
				out_str += "\x1b[38;5;3mUnknown\x1b[0m";
				break;
			case handle::Noun:
				out_str += "\x1b[38;5;43mNoun\x1b[0m";
				break;
			case handle::Verb:
				out_str += "\x1b[38;5;43mVerb\x1b[0m";
				break;
			case handle::Pronoun:
				out_str += "\x1b[38;5;43mPronoun\x1b[0m";
				break;
			case handle::Adverb:
				out_str += "\x1b[38;5;43mAdverb\x1b[0m";
				break;
			case handle::Adjective:
				out_str += "\x1b[38;5;43mAdjective\x1b[0m";
				break;
			case handle::Preposition:
				out_str += "\x1b[38;5;43mPrepostion\x1b[0m";
				break;
			case handle::Conjunction:
				out_str += "\x1b[38;5;43mConjunction\x1b[0m";
				break;
			case handle::Interjection:
				out_str += "\x1b[38;5;43mInterjection\x1b[0m";
				break;
			default:
				break;
			}
		};

		for (uint32 i = 0; i < words.size(); i++) {
			util::IntToStr(out_str, i + 1);

			out_str += "- [\x1b[38;5;231m";
			out_str += *words.at(i).data;
			out_str += "\x1b[0m]: ";
			for (uint32 j = 0; j < words.at(i).type.size(); j++) {
				printWordType(words.at(i).type.at(j));
				if (j != words.at(i).type.size() - 1) { // If not last index
					out_str += ", ";
				}
				else {
					out_str += '.';
				}
			}
			out_str += '\n';
		}
	}

	void MainHandler::PrintResult(const WordType type, const int32& result, const String& str) {
		switch (type)
		{
		case Noun:
			switch (result)
			{
			case 1:
				Log::Info("Noun has been found:\n") << str.Chars();
				break;
			case 2:
				Log::Warning("The input is empty.\n");
				break;
			case -1:
				Log::Warning("No noun has been found.\n");
				break;
			case -2:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -3:
				Log::Warning("The first character of the noun cannot be hyphen.\n");
				break;
			case -4:
				Log::Warning("The noun cannot be less than 1 characters.\n");
				break;
			case -5:
				Log::Warning("Character size exceeds ") << NOUN_CHAR_SIZE << " size.\n";
				break;
			default:
				break;
			}
			return;
		case Verb:
			switch (result)
			{
			case 1:
				Log::Info("Verb has been found:\n") << str.Chars();
				break;
			case 2:
				Log::Warning("The input is empty.\n");
				break;
			case -1:
				Log::Warning("No verb has been found.\n");
				break;
			case -2:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -3:
				Log::Warning("The first character of the verb cannot be hyphen.\n");
				break;
			case -4:
				Log::Warning("The verb cannot be less than 2 characters.\n");
				break;
			case -5:
				Log::Warning("Character size exceeds ") << VERB_CHAR_SIZE << " size.\n";
				break;
			default:
				break;
			}
			return;
		case Pronoun:
			switch (result)
			{
			case 1:
				Log::Info("Pronoun has been found:\n") << str.Chars();
				break;
			case -1:
				Log::Warning("No pronoun has been found.\n");
				break;
			case -4:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -2:
				Log::Warning("Character size exceeds ") << WORD_CHAR_SIZE << " size.\n";
				break;
			case -3:
				Log::Warning("The input is empty\n");
				break;
			default:
				break;
			}
			return;
		case Adverb:
			switch (result)
			{
			case 1:
				Log::Info("Adverb has been found:\n") << str.Chars();
				break;
			case -1:
				Log::Warning("No adverb has been found.\n");
				break;
			case -4:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -2:
				Log::Warning("Character size exceeds ") << WORD_CHAR_SIZE << " size.\n";
				break;
			case -3:
				Log::Warning("The input is empty\n");
				break;
			default:
				break;
			}
			return;
		case Adjective:
			switch (result)
			{
			case 1:
				Log::Info("Adjective has been found:\n") << str.Chars();
				break;
			case -1:
				Log::Warning("No adjective has been found.\n");
				break;
			case -4:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -2:
				Log::Warning("Character size exceeds ") << WORD_CHAR_SIZE << " size.\n";
				break;
			case -3:
				Log::Warning("The input is empty\n");
				break;
			default:
				break;
			}
			return;
		case Preposition:
			switch (result)
			{
			case 1:
				Log::Info("Preposition has been found:\n") << str.Chars();
				break;
			case -1:
				Log::Warning("No preposition has been found.\n");
				break;
			case -4:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -2:
				Log::Warning("Character size exceeds ") << WORD_CHAR_SIZE << " size.\n";
				break;
			case -3:
				Log::Warning("The input is empty\n");
				break;
			default:
				break;
			}
			return;
		case Conjunction:
			switch (result)
			{
			case 1:
				Log::Info("Conjunction has been found:\n") << str.Chars();
				break;
			case -1:
				Log::Warning("No conjunction has been found.\n");
				break;
			case -4:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -2:
				Log::Warning("Character size exceeds ") << WORD_CHAR_SIZE << " size.\n";
				break;
			case -3:
				Log::Warning("The input is empty\n");
				break;
			default:
				break;
			}
			return;
		case Interjection:
			switch (result)
			{
			case 1:
				Log::Info("Interjection has been found:\n") << str.Chars();
				break;
			case -1:
				Log::Warning("No interjection has been found.\n");
				break;
			case -4:
				Log::Warning("The input contains some characters which is not alphabetic.\n");
				break;
			case -2:
				Log::Warning("Character size exceeds ") << WORD_CHAR_SIZE << " size.\n";
				break;
			case -3:
				Log::Warning("The input is empty\n");
				break;
			default:
				break;
			}
			return;
		default:
			return;
		}
	}

}