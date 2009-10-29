/* 
 * Copyright 2008, 2009 Nicolas Maingot
 * 
 * This file is part of CSSMatch.
 * 
 * CSSMatch is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CSSMatch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CSSMatch; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Portions of this code are also Copyright � 1996-2005 Valve Corporation, All rights reserved
 */

#include "I18nManager.h"

#include "../configuration/TranslationFile.h"
#include "../common/common.h"
#include "../plugin/SimplePlugin.h"

#include "convar.h"
#include "eiface.h"

#include <vector>

using namespace cssmatch;

using std::string;
using std::map;
using std::vector;

map<string, string> I18nManager::WITHOUT_PARAMETERS;

I18nManager::I18nManager(IVEngineServer * engine, ConVar * defaultLang)
	: UserMessagesManager(engine), defaultLanguage(defaultLang)
{
}

I18nManager::~I18nManager()
{
	map<string,TranslationFile *>::iterator itLanguages = languages.begin();
	map<string,TranslationFile *>::iterator lastLanguages = languages.end();
	while(itLanguages != lastLanguages)
	{
		delete itLanguages->second;

		itLanguages++;
	}
}

string I18nManager::getDefaultLanguage() const
{
	return defaultLanguage->GetString();
}

TranslationFile * I18nManager::getTranslationFile(const string & language)
{
	TranslationFile * translationSet = NULL;

	// Is the language already used/known (in the cache) ?
	map<string,TranslationFile *>::iterator itLanguages = languages.find(language);
	map<string,TranslationFile *>::iterator lastLanguages = languages.end();
	if (itLanguages == lastLanguages)
	{
		// No, put these translations in the cache
		try
		{
			translationSet = new TranslationFile(TRANSLATIONS_FOLDER + language + ".txt");
			
			languages[language] = translationSet;
		}
		catch(const ConfigurationFileException & e)
		{
			// The file was not found, we'll use the default language instead
			std::string defaultLanguageName = defaultLanguage->GetString();
			std::map<std::string,TranslationFile *>::iterator itDefault = languages.find(defaultLanguageName);
			if (itDefault == lastLanguages)
			{
				try
				{
					translationSet = new TranslationFile(TRANSLATIONS_FOLDER + defaultLanguageName + ".txt");
					languages[defaultLanguageName] = translationSet;
				}
				catch(const ConfigurationFileException & e)
				{
					print(__FILE__,__LINE__,"ERROR ! Default translation file not found !");
				}
			}
			else
				translationSet = itDefault->second;
		}
	}
	else
	{
		// Yes, the translation is already available
		translationSet = itLanguages->second;
	}

	return translationSet;
}

string I18nManager::getTranslation(	const string & lang,
									const string & keyword,
									map<string,string> & parameters)
{
	string message;

	// We have to get the translations corresponding to this language
	TranslationFile * translation = getTranslationFile(lang);
	
	if (translation != NULL)
	{
		try
		{
			message = (*translation)[keyword]; // copying it, because we will replace the parameters

			// Relace the parameters // FIXME : unnecessarily call as many times as recipients
			map<string,string>::const_iterator itParameters = parameters.begin();
			map<string,string>::const_iterator lastParameters = parameters.end();
			while(itParameters != lastParameters)
			{
				const string * parameter = &itParameters->first;
				size_t parameterSize = parameter->size();
				const string * value = &itParameters->second;
				size_t valueSize = value->size();

				// One option can be found multiple times
				size_t iParam = message.find(*parameter);
				while(iParam != string::npos)
				{
					message.replace(iParam,parameterSize,*value,0,valueSize);
					iParam = message.find(*parameter);
				}

				itParameters++;
			}
		}
		catch(const TranslationException & e)
		{
			printException(e,__FILE__,__LINE__);
		}
	}
	else
	{
		message = "Missing translation, please update your translation files";
	}

	return message;
}

void I18nManager::i18nChatSay(	RecipientFilter & recipients,
								const string & keyword,
								int playerIndex,
								map<string,string> & parameters)
{
	const vector<int> * recipientVector = recipients.getVector();
	vector<int>::const_iterator itIndex = recipientVector->begin();
	vector<int>::const_iterator badIndex = recipientVector->end();
	while(itIndex != badIndex)
	{
		RecipientFilter thisRecipient;
		thisRecipient.addRecipient(*itIndex);

		string langage = engine->GetClientConVarValue(*itIndex,"cl_language");
		string message = getTranslation(langage,keyword,parameters);

		chatSay(thisRecipient,message,playerIndex);

		itIndex++;
	}
}

void I18nManager::i18nChatWarning(	RecipientFilter & recipients, 
									const string & keyword,
									map<string,string> & parameters)
{
	const vector<int> * recipientVector = recipients.getVector();
	vector<int>::const_iterator itIndex = recipientVector->begin();
	vector<int>::const_iterator badIndex = recipientVector->end();
	while(itIndex != badIndex)
	{
		RecipientFilter thisRecipient;
		thisRecipient.addRecipient(*itIndex);

		string langage = engine->GetClientConVarValue(*itIndex,"cl_language");
		string message = getTranslation(langage,keyword,parameters);

		chatWarning(thisRecipient,message);

		itIndex++;
	}
}

void I18nManager::i18nPopupSay(	RecipientFilter & recipients,
								const std::string & keyword,
								int lifeTime,
								PopupSensitivityFlags flags,
								map<string,string> & parameters)
{
	const vector<int> * recipientVector = recipients.getVector();
	vector<int>::const_iterator itIndex = recipientVector->begin();
	vector<int>::const_iterator badIndex = recipientVector->end();
	while(itIndex != badIndex)
	{
		RecipientFilter thisRecipient;
		thisRecipient.addRecipient(*itIndex);

		string langage = engine->GetClientConVarValue(*itIndex,"cl_language");
		string message = getTranslation(langage,keyword,parameters);

		popupSay(thisRecipient,message,lifeTime,flags);

		itIndex++;
	}
}

void I18nManager::i18nHintSay(	RecipientFilter & recipients,
								const string & keyword,
								map<string,string> & parameters)
{
	const vector<int> * recipientVector = recipients.getVector();
	vector<int>::const_iterator itIndex = recipientVector->begin();
	vector<int>::const_iterator badIndex = recipientVector->end();
	while(itIndex != badIndex)
	{
		RecipientFilter thisRecipient;
		thisRecipient.addRecipient(*itIndex);

		string langage = engine->GetClientConVarValue(*itIndex,"cl_language");
		string message = getTranslation(langage,keyword,parameters);

		hintSay(thisRecipient,message);

		itIndex++;
	}
}

void I18nManager::i18nCenterSay(RecipientFilter & recipients,
								const string & keyword,
								map<string,string> & parameters)
{
	const vector<int> * recipientVector = recipients.getVector();
	vector<int>::const_iterator itIndex = recipientVector->begin();
	vector<int>::const_iterator badIndex = recipientVector->end();
	while(itIndex != badIndex)
	{
		RecipientFilter thisRecipient;
		thisRecipient.addRecipient(*itIndex);

		string langage = engine->GetClientConVarValue(*itIndex,"cl_language");
		string message = getTranslation(langage,keyword,parameters);

		centerSay(thisRecipient,message);

		itIndex++;
	}
}

void I18nManager::i18nConsoleSay(	RecipientFilter & recipients,
									const string & keyword,
									map<string,string> & parameters)
{
	const vector<int> * recipientVector = recipients.getVector();
	vector<int>::const_iterator itIndex = recipientVector->begin();
	vector<int>::const_iterator badIndex = recipientVector->end();
	while(itIndex != badIndex)
	{
		RecipientFilter thisRecipient;
		thisRecipient.addRecipient(*itIndex);

		string langage = engine->GetClientConVarValue(*itIndex,"cl_language");
		string message = getTranslation(langage,keyword,parameters);

		consoleSay(thisRecipient,message);

		itIndex++;
	}
}

TimerI18nChatSay::TimerI18nChatSay(	I18nManager * manager,
									float date,
									RecipientFilter & recip,
									const string & key,
									int pIndex,
									map<string,string> & param)
	: BaseTimer(date), i18n(manager), recipients(recip), keyword(key), playerIndex(pIndex), parameters(param)
{
}

void TimerI18nChatSay::execute()
{
	i18n->i18nChatSay(recipients,keyword,playerIndex,parameters);
}

TimerI18nPopupSay::TimerI18nPopupSay(	I18nManager * manager,
										float date,
										RecipientFilter & recip,
										const string & key,
										int life,
										PopupSensitivityFlags fl,
										map<string,string> & param)
	:	BaseTimer(date), i18n(manager), recipients(recip), keyword(key), lifeTime(life), flags(fl),
		parameters(param)
{
}

void TimerI18nPopupSay::execute()
{
	i18n->i18nPopupSay(recipients,keyword,lifeTime,flags,parameters);
}