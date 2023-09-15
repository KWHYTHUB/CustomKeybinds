#pragma once

#include <Sapphire/DefaultInclude.hpp>
#include <Sapphire/utils/MiniFunction.hpp>
#include <Sapphire/utils/cocos.hpp>
#include <Sapphire/loader/Mod.hpp>
#include <Sapphire/loader/Event.hpp>
#include <cocos2d.h>

#ifdef SAPPHIRE_IS_WINDOWS
    #ifdef HJFOD_CUSTOM_KEYBINDS_EXPORTING
        #define CUSTOM_KEYBINDS_DLL __declspec(dllexport)
    #else
        #define CUSTOM_KEYBINDS_DLL __declspec(dllimport)
    #endif
#else
    #define CUSTOM_KEYBINDS_DLL
#endif

struct BindSaveData;

namespace keybinds {
    class Bind;
    class BindManager;

    using DeviceID = std::string;

    /**
     * Base class for implementing bindings for different input devices
     */
    class CUSTOM_KEYBINDS_DLL Bind : public cocos2d::CCObject {
    protected:
        friend class BindManager;

    public:
        /**
         * Get the hash for this bind
         */
        virtual size_t getHash() const = 0;
        /**
         * Check if this bind is equal to another. By default compares hashes
         */
        virtual bool isEqual(Bind* other) const;
        /**
         * Get the bind's representation as a human-readable string
         */
        virtual std::string toString() const = 0;
        virtual cocos2d::CCNode* createLabel() const;
        virtual DeviceID getDeviceID() const = 0;
        virtual json::Value save() const = 0;

        virtual ~Bind() = default;

        cocos2d::CCNodeRGBA* createBindSprite() const;
    };

    enum class Modifier : unsigned int {
        None        = 0b0000,
        Control     = 0b0001,
        Shift       = 0b0010,
        Alt         = 0b0100,
        Command     = 0b1000,
    };
    CUSTOM_KEYBINDS_DLL Modifier operator|(Modifier const& a, Modifier const& b);
    CUSTOM_KEYBINDS_DLL Modifier operator|=(Modifier& a, Modifier const& b);
    CUSTOM_KEYBINDS_DLL bool operator&(Modifier const& a, Modifier const& b);

    CUSTOM_KEYBINDS_DLL std::string keyToString(cocos2d::enumKeyCodes key);
    CUSTOM_KEYBINDS_DLL bool keyIsModifier(cocos2d::enumKeyCodes key);
    CUSTOM_KEYBINDS_DLL bool keyIsController(cocos2d::enumKeyCodes key);

    class CUSTOM_KEYBINDS_DLL Keybind final : public Bind {
    protected:
        cocos2d::enumKeyCodes m_key;
        Modifier m_modifiers;

    public:
        static Keybind* create(cocos2d::enumKeyCodes key, Modifier modifiers = Modifier::None);
        static Keybind* parse(json::Value const&);

        cocos2d::enumKeyCodes getKey() const;
        Modifier getModifiers() const;

        size_t getHash() const override;
        bool isEqual(Bind* other) const override;
        std::string toString() const override;
        DeviceID getDeviceID() const override;
        json::Value save() const override;
    };

    class CUSTOM_KEYBINDS_DLL ControllerBind final : public Bind {
    protected:
        cocos2d::enumKeyCodes m_button;
    
    public:
        static ControllerBind* create(cocos2d::enumKeyCodes button);
        static ControllerBind* parse(json::Value const&);

        cocos2d::enumKeyCodes getButton() const;

        size_t getHash() const override;
        bool isEqual(Bind* other) const override;
        std::string toString() const override;
        cocos2d::CCNode* createLabel() const override;
        DeviceID getDeviceID() const override;
        json::Value save() const override;
    };

    struct CUSTOM_KEYBINDS_DLL BindHash {
        sapphire::Ref<Bind> bind;
        BindHash(Bind* bind);
        bool operator==(BindHash const& other) const;
    };
}

namespace std {
    template <>
    struct hash<keybinds::BindHash> {
        CUSTOM_KEYBINDS_DLL std::size_t operator()(keybinds::BindHash const&) const;
    };
}

namespace keybinds {
    class BindManager;
    class InvokeBindFilter;

    using ActionID = std::string;
    class CUSTOM_KEYBINDS_DLL Category final {
        std::string m_value;
    
    public:
        Category() = default;
        Category(const char* path);
        Category(std::string const& path);
        std::vector<std::string> getPath() const;
        std::optional<Category> getParent() const;
        bool hasParent(Category const& parent) const;
        std::string toString() const;

        bool operator==(Category const&) const;

        static constexpr auto PLAY { "Play" };
        static constexpr auto EDITOR { "Editor" };
        static constexpr auto GLOBAL { "Global" };
        static constexpr auto EDITOR_UI { "Editor/UI" };
        static constexpr auto EDITOR_MODIFY { "Editor/Modify" };
        static constexpr auto EDITOR_MOVE { "Editor/Move" };
    };

    class CUSTOM_KEYBINDS_DLL BindableAction {
    protected:
        ActionID m_id;
        std::string m_name;
        std::string m_description;
        sapphire::Mod* m_owner;
        std::vector<sapphire::Ref<Bind>> m_defaults;
        Category m_category;
        bool m_repeatable;
    
    public:
        ActionID getID() const;
        std::string getName() const;
        std::string getDescription() const;
        sapphire::Mod* getMod() const;
        std::vector<sapphire::Ref<Bind>> getDefaults() const;
        Category getCategory() const;
        bool isRepeatable() const;

        BindableAction() = default;
        BindableAction(
            ActionID const& id,
            std::string const& name,
            std::string const& description = "",
            std::vector<sapphire::Ref<Bind>> const& defaults = {},
            Category const& category = Category(),
            bool repeatable = true, 
            sapphire::Mod* owner = sapphire::Mod::get()
        );
    };

    class CUSTOM_KEYBINDS_DLL InvokeBindEvent : public sapphire::Event {
    protected:
        ActionID m_id;
        bool m_down;

        friend class BindManager;
        friend class InvokeBindFilter;

    public:
        InvokeBindEvent(ActionID const& id, bool down);
        ActionID getID() const;
        bool isDown() const;
    };

    class CUSTOM_KEYBINDS_DLL InvokeBindFilter : public sapphire::EventFilter<InvokeBindEvent> {
    protected:
        cocos2d::CCNode* m_target;
        ActionID m_id;

    public:
        using Callback = sapphire::ListenerResult(InvokeBindEvent*);
        
        sapphire::ListenerResult handle(sapphire::utils::MiniFunction<Callback> fn, InvokeBindEvent* event);
        InvokeBindFilter(cocos2d::CCNode* target, ActionID const& id);
    };

    class CUSTOM_KEYBINDS_DLL PressBindEvent : public sapphire::Event {
    protected:
        Bind* m_bind;
        bool m_down;
    
    public:
        PressBindEvent(Bind* bind, bool down);
        Bind* getBind() const;
        bool isDown() const;
    };

    class CUSTOM_KEYBINDS_DLL PressBindFilter : public sapphire::EventFilter<PressBindEvent> {
    public:
        using Callback = sapphire::ListenerResult(PressBindEvent*);
        
        sapphire::ListenerResult handle(sapphire::utils::MiniFunction<Callback> fn, PressBindEvent* event);
        PressBindFilter();
    };

    class CUSTOM_KEYBINDS_DLL DeviceEvent : public sapphire::Event {
    protected:
        DeviceID m_id;
        bool m_attached;
    
    public:
        DeviceEvent(DeviceID const& id, bool attached);
        DeviceID getID() const;
        bool wasAttached() const;
        bool wasDetached() const;
    };

    class CUSTOM_KEYBINDS_DLL DeviceFilter : public sapphire::EventFilter<DeviceEvent> {
    protected:
        std::optional<DeviceID> m_id;

    public:
        using Callback = void(DeviceEvent*);

        sapphire::ListenerResult handle(sapphire::utils::MiniFunction<Callback> fn, DeviceEvent* event);
        DeviceFilter(std::optional<DeviceID> id = std::nullopt);
    };

    struct CUSTOM_KEYBINDS_DLL RepeatOptions {
        bool enabled = false;
        size_t rate = 300;
        size_t delay = 500;
    };
    
    using BindParser = std::function<Bind*(json::Value const&)>;

    class CUSTOM_KEYBINDS_DLL BindManager : public cocos2d::CCObject {
    // has to inherit from CCObject for scheduler
    public:
        using DevicelessActions = std::unordered_map<ActionID, std::set<json::Value>>;

    protected:
        struct ActionData {
            BindableAction definition;
            RepeatOptions repeat;
        };
        
        std::unordered_map<BindHash, std::vector<ActionID>> m_binds;
        std::unordered_map<DeviceID, DevicelessActions> m_devicelessBinds;
        std::unordered_map<DeviceID, BindParser> m_devices;
        std::vector<std::pair<ActionID, ActionData>> m_actions;
        std::vector<Category> m_categories;
        sapphire::EventListener<PressBindFilter> m_listener =
            sapphire::EventListener<PressBindFilter>(this, &BindManager::onDispatch);
        std::vector<std::pair<ActionID, float>> m_repeating;

        BindManager();

        sapphire::ListenerResult onDispatch(PressBindEvent* event);
        void onRepeat(float dt);
        void repeat(ActionID const& action);
        void unrepeat(ActionID const& action);

        bool loadActionBinds(ActionID const& action);
        void saveActionBinds(ActionID const& action);

        friend class InvokeBindFilter;
        friend struct json::Serialize<BindSaveData>;

    public:
        static BindManager* get();
        void save();

        void attachDevice(DeviceID const& device, BindParser parser);
        void detachDevice(DeviceID const& device);

        json::Value saveBind(Bind* bind) const;
        Bind* loadBind(json::Value const& json) const;

        bool registerBindable(BindableAction const& action, ActionID const& after = "");
        void removeBindable(ActionID const& action);
        std::optional<BindableAction> getBindable(ActionID const& action) const;
        std::vector<BindableAction> getAllBindables() const;
        std::vector<BindableAction> getBindablesIn(Category const& category, bool sub = false) const;
        std::vector<BindableAction> getBindablesFor(Bind* bind) const;
        std::vector<Category> getAllCategories() const;
        /**
         * Add a new bindable category. If the category is a subcategory (its 
         * ID has a slash, like "Editor/Modify"), then all its parent 
         * categories are inserted aswell, and the subcategory is added after 
         * its parent's last subcategory
         * @param category The category to add. Specify a subcategory by 
         * including a slash in the name (like "Editor/Modify")
         */
        void addCategory(Category const& category);
        /**
         * @note Also removes all the bindables in this category
         */
        void removeCategory(Category const& category);

        void addBindTo(ActionID const& action, Bind* bind);
        void removeBindFrom(ActionID const& action, Bind* bind);
        void removeAllBindsFrom(ActionID const& action);
        void resetBindsToDefault(ActionID const& action);
        bool hasDefaultBinds(ActionID const& action) const;
        std::vector<sapphire::Ref<Bind>> getBindsFor(ActionID const& action) const;

        std::optional<RepeatOptions> getRepeatOptionsFor(ActionID const& action);
        void setRepeatOptionsFor(ActionID const& action, RepeatOptions const& options);
        void stopAllRepeats();
    };
}
