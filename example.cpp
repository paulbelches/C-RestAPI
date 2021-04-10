#include <iostream>
#include <vector>
#include <map>
#include "rapidjson/document.h"

std::map<std::string, std::string> mapForAttributeThatMatchesName(const rapidjson::Value& attributes, const std::string& findMemberName, const std::string& findMemberValue, const std::vector<std::string>& keysToRetrieve) {

    std::map<std::string, std::string> result;
    for (rapidjson::Value::ConstValueIterator itr = attributes.Begin(); itr != attributes.End(); ++itr) {

        const rapidjson::Value::ConstMemberIterator currentAttribute = itr->FindMember(findMemberName.c_str());
        if (currentAttribute != itr->MemberEnd() && currentAttribute->value.IsString()) {

            if (currentAttribute->value == findMemberValue.c_str()) {

                for (auto &keyToRetrieve : keysToRetrieve) {

                    const rapidjson::Value::ConstMemberIterator currentAttributeToReturn = itr->FindMember(keyToRetrieve.c_str());
                    if (currentAttributeToReturn != itr->MemberEnd() && currentAttributeToReturn->value.IsString()) {

                        result[keyToRetrieve] = currentAttributeToReturn->value.GetString();
                    }
                }
                return result;
            }
        }
    }
    return result;
}

const rapidjson::Value& attributes = config["attributes"];
assert(attributes.IsArray());

std::vector<std::string> keysToRetrieve = {"maximumValue", "minimumValue"};
std::map<std::string, std::string> mapForResult = mapForAttributeThatMatchesName(attributes, "name", "mass", keysToRetrieve);
for (auto &mapItem : mapForResult) {

    std::cout << mapItem.first << ":" << mapItem.second << "\n";
}