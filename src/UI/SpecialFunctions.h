#pragma once

#include <memory>
#include "Funscript.h"

// ATTENTION: no reordering
enum SpecialFunctions : int32_t
{
	RANGE_EXTENDER,
	RAMER_DOUGLAS_PEUCKER, // simplification
	CUSTOM_LUA_FUNCTIONS,
	TOTAL_FUNCTIONS_COUNT
};

class FunctionBase {
protected:
	inline Funscript& ctx() noexcept;
public:
	virtual ~FunctionBase() noexcept {}
	virtual void DrawUI() noexcept = 0;
};

class FunctionRangeExtender : public FunctionBase 
{
	int32_t rangeExtend = 0;
	bool createUndoState = true;
public:
	FunctionRangeExtender() noexcept;
	virtual ~FunctionRangeExtender() noexcept;
	void SelectionChanged(union SDL_Event& ev) noexcept;
	virtual void DrawUI() noexcept override;
};

class RamerDouglasPeucker : public FunctionBase
{
	float epsilon = 0.0f;
	float averageDistance = 0.f;
	bool createUndoState = true;
public:
	RamerDouglasPeucker() noexcept;
	virtual ~RamerDouglasPeucker() noexcept;
	void SelectionChanged(union SDL_Event& ev) noexcept;
	virtual void DrawUI() noexcept override;
};

class CustomLua : public FunctionBase 
{
public:
	struct LuaScript {
		std::string name;
		std::string absolutePath;

		struct Settings {
			~Settings();

			struct Value {
				enum class Type : int32_t {
					Nil, // invalid
					Bool,
					//Integer, // everything is a float for now
					Float,
					String
				};

				std::string name;
				Type type = Type::Nil;
				int32_t offset = 0;
			};

			// name of settings table
			std::string name = "Settings"; 
			std::vector<uint8_t> buffer;
			std::vector<Value> values;

			inline void Free() noexcept {
				if (buffer.size() > 0) {
					for (auto&& value : values) {
						if (value.type == Settings::Value::Type::String) {
							std::string* str = (std::string*)&buffer[value.offset];
							str->~basic_string(); // requires explicit destruction since it might have heap allocated
						}
					}
				}
				buffer.clear();
				values.clear();
			}
		} settings;
	};
private:
	std::vector<LuaScript> scripts;
	bool createUndoState = true;
	bool showSettings = false;

	void updateScripts() noexcept;
	void resetVM() noexcept;
	void runScript(LuaScript* script, bool dry_run = false) noexcept;
public:
	CustomLua() noexcept;
	virtual ~CustomLua() noexcept;
	void SelectionChanged(union SDL_Event& ev) noexcept;
	virtual void DrawUI() noexcept override;

	void HandleBinding(class Binding* binding) noexcept;
	void RunScript(const std::string& name) noexcept;
};

class SpecialFunctionsWindow {
	CustomLua lua;
	FunctionBase* function = nullptr;
public:
	static constexpr const char* SpecialFunctionsId = "Special functions";
	SpecialFunctionsWindow() noexcept;
	void SetFunction(SpecialFunctions functionEnum) noexcept;
	void ShowFunctionsWindow(bool* open) noexcept;
};