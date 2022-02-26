#pragma once

#include <memory>
#include <utility>

#include "IMCommon.hpp"

namespace MWD::Config {
    /********************************************************
     配置元素
    *********************************************************/
    class IMConfigElement {
    public:
        IMConfigElement(MString key, MString value,
                        std::shared_ptr<std::vector<std::basic_string<char, std::char_traits<char>, std::allocator<char>>, std::allocator<std::basic_string<char, std::char_traits<char>, std::allocator<char>>>>> values)
                : m_key(std::move(key)), m_value(std::move(value)), m_values(std::move(values)) {}

    public:
        MString m_key;
        MString m_value;
        std::shared_ptr<Vector<MString>> m_values;
    };

    /********************************************************
     配置文件加载器
    *********************************************************/
    class IMConfigFileLoader {
    public:
        explicit IMConfigFileLoader(std::ifstream& ifs,
                                    std::shared_ptr<Map<MString, std::shared_ptr<IMConfigElement>>> map) : m_ifs(ifs),
                                                                                                           m_map(std::move(
                                                                                                                   map)) {}

        virtual ~IMConfigFileLoader() = default;

    public:
        virtual void LoadFile(MString file) = 0;

        virtual MString FindValue(const MString& key) = 0;

        virtual std::shared_ptr<Vector<MString>> FindValues(const MString& key) = 0;

        virtual void AddKeyValue(MString key, MString value, std::shared_ptr<Vector<MString>> values) = 0;

        virtual void SetKeyValueMap(std::shared_ptr<Map<MString, std::shared_ptr<IMConfigElement>>> map) {
            m_map = std::move(map);
        }

        virtual void SetInputFileStream(std::ifstream& ifs) {
            m_ifs = std::move(ifs);
        }

    protected:
        std::shared_ptr<Map<MString, std::shared_ptr<IMConfigElement>>> m_map;
        std::ifstream& m_ifs;
    };

    /********************************************************
     yaml配置文件加载器实现
    *********************************************************/
    class MYamlConfigFileLoader : public IMConfigFileLoader {
    public:
        MYamlConfigFileLoader(std::ifstream& ifs, std::shared_ptr<Map<MString, std::shared_ptr<IMConfigElement>>> map)
                : IMConfigFileLoader(ifs, std::move(map)) {

        }

        ~MYamlConfigFileLoader() override {
            if (m_ifs.is_open()) {
                m_ifs.close();
            }
        }

    public:
        void LoadFile(MString file) override {
            m_ifs.open(file);


        }

        MString FindValue(const MString& key) override {

        }

        std::shared_ptr<Vector<MString>> FindValues(const MString& key) override {

        }

        void AddKeyValue(MString key, MString value,
                         std::shared_ptr<Vector<MString>> values) override {

        }
    };

    /********************************************************
     properties配置文件加载器实现
    *********************************************************/
    class MPropertiesConfigFileLoader : public IMConfigFileLoader {
    public:
        MPropertiesConfigFileLoader(std::ifstream& ifs,
                                    std::shared_ptr<Map<MString, std::shared_ptr<IMConfigElement>>> map)
                : IMConfigFileLoader(ifs, std::move(map)) {

        }

        ~MPropertiesConfigFileLoader() override {
            if (m_ifs.is_open()) {
                m_ifs.close();
            }
        }

    public:
        void LoadFile(MString file) override {
            m_ifs.open(file);

            if (!m_ifs) {
                return;
            }

            MString temp;
            while (std::getline(m_ifs, temp)) {
                temp.erase(0, temp.find_first_not_of(" \t"));
                temp.erase(temp.find_last_not_of(" \t") + 1);

                if (temp.empty()) continue;

                if (temp[0] == '#') continue;

                MString key, value;
                for (MInt i = 0; i < temp.length(); ++i) {
                    if (temp[i] == '=') {
                        key = temp.substr(0, i);
                        value = temp.substr(i + 1, temp.length() - i - 1);

                        key.erase(0, key.find_first_not_of(" \t"));
                        key.erase(key.find_last_not_of(" \t") + 1);

                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t") + 1);
                        break;
                    }
                }

                if (!key.empty()) {
                    std::shared_ptr<Vector<MString>> vec = std::make_shared<Vector<MString>>();
                    if (value[0] == '"' && value[value.length()] == '"') {
                        value = value.substr(1, value.size() - 2);
                        vec = MWD::GM::IMTools::StringSplit(value, ',');
                        if (!vec->empty()) {
                            value = vec->front();
                        }
                    } else {
                        vec->push_back(value);
                    }

                    std::shared_ptr<IMConfigElement> ele = std::make_shared<IMConfigElement>(key, value, vec);
                    m_map->insert(std::make_pair(key, ele));
                }
            }
        }

        MString FindValue(const MString& key) override {
            auto item = m_map->find(key);
            if (item != m_map->end()) {
                return item->second->m_value;
            }
            return "";
        }

        std::shared_ptr<Vector<MString>> FindValues(const MString& key) override {
            auto item = m_map->find(key);
            if (item != m_map->end()) {
                if (item->second->m_values) {
                    return item->second->m_values;
                }
            }
            return nullptr;
        }

        void AddKeyValue(MString key, MString value,
                         std::shared_ptr<Vector<MString>> values) override {
            std::shared_ptr<IMConfigElement> ele = std::make_shared<IMConfigElement>(key, value, std::move(values));
            m_map->insert(std::make_pair(key, ele));
        }
    };

    /********************************************************
     配置类
    *********************************************************/
    class IMConfig {
    public:
        explicit IMConfig() = default;

        explicit IMConfig(MString file) : m_file(std::move(file)) {}

    public:
        void SetLoader(std::shared_ptr<IMConfigFileLoader> loader) {
            m_pCfgLoader = std::move(loader);
        }

        void SetFilePath(MString file) {
            m_file = std::move(file);
        }

        void Init() {
            MString aa = m_file.substr(m_file.size() - 11, 11);
            if (!m_file.empty() && m_file.substr(m_file.size() - 11, 11) == ".properties")
                m_pCfgLoader->LoadFile(m_file);
            else
                throw std::invalid_argument("Config file load failed!");
        }

        MString FindValue(const MString& key) {
            return m_pCfgLoader->FindValue(key);
        }

        std::shared_ptr<Vector<MString>> FindValues(const MString& key) {
            return m_pCfgLoader->FindValues(key);
        }

        void AddKeyValue(MString key, MString value, std::shared_ptr<Vector<MString>> values) {
            m_pCfgLoader->AddKeyValue(std::move(key), std::move(value), std::move(values));
        }

    private:
        std::shared_ptr<IMConfigFileLoader> m_pCfgLoader;
        MString m_file;
    };

    /********************************************************
     配置资源类
    *********************************************************/
    class IMConfigResourcesManager {
    public:
        static std::shared_ptr<IMConfig> GetConfigCore() {
            return MWD::GM::IMSingletonPtr<IMConfig>::GetInstance();
        }

        static std::shared_ptr<MPropertiesConfigFileLoader> GetPropertiesConfigLoader() {
            static std::ifstream configInputStream;
            static std::shared_ptr<Map<MString, std::shared_ptr<IMConfigElement>>> configMap = std::make_shared<Map<MString, std::shared_ptr<IMConfigElement>>>();
            return std::make_shared<MPropertiesConfigFileLoader>(configInputStream, configMap);
        }

        static void SetFileConfigLoader(std::shared_ptr<IMConfigFileLoader> loader) {
            GetConfigCore()->SetLoader(std::move(loader));
        }

        static void LoadConfig(MString file) {
            GetConfigCore()->SetFilePath(std::move(file));
            GetConfigCore()->Init();
        }

        static MString FindValue(const MString& key) {
            return GetConfigCore()->FindValue(key);
        }

        static std::shared_ptr<Vector<MString>> FindValues(const MString& key) {
            return GetConfigCore()->FindValues(key);
        }
    };

#define IM_CONFIG_SET_DEFAULT_FILE_LOADER \
    MWD::Config::IMConfigResourcesManager::SetFileConfigLoader(MWD::Config::IMConfigResourcesManager::GetPropertiesConfigLoader());

#define IM_CONFIG_GET_ARG(KEY) \
    MWD::Config::IMConfigResourcesManager::FindValue(KEY);

#define IM_CONFIG_GET_ARGS(KEY) \
    MWD::Config::IMConfigResourcesManager::FindValues(KEY);

#define IM_CONFIG_STARTUP(FILE_PATH) \
    MWD::Config::IMConfigResourcesManager::LoadConfig(FILE_PATH);

}





