#include "tcApp.h"

void tcApp::setup() {
    lua = tcx_lua.getLuaState();

    lua->set_function("testJson", [&](){ testJson(); });
    lua->set_function("testXml", [&](){ testXml(); });

    std::string setupLuaSource = R"LUA(
        setWindowTitle("jsonXmlExample")
        tcSetConsoleLogLevel(LogLevel.Verbose)

        messages = {}

        function addMessage(msg)
            table.insert(messages, msg)
            -- Remove old messages to fit on screen
            if #messages > 35 then
                table.remove(messages, 1)
            end
        end

        addMessage("=== JSON/XML Example ===")
        addMessage("");
        addMessage("Press 'j' to test JSON")
        addMessage("Press 'x' to test XML")
        addMessage("")
    )LUA";

    sol::optional<sol::error> result = lua->safe_script(setupLuaSource);
	if (result.has_value()) {
		std::cerr << "Lua execution failed: "
		          << result.value().what() << std::endl;
	}
}

void tcApp::update() {

}

void tcApp::draw() {
    if(isFirstDraw){
        std::string drawLuaSource = R"LUA(
            function draw()
                clear(0.12, 1.0)

                setColor(1.0, 1.0)
                local y = 20
                for k, msg in ipairs(messages) do
                    drawBitmapString(msg, 20, y, true)
                    y = y + 15
                end
            end
        )LUA";

        lua->script(drawLuaSource);

        isFirstDraw = false;
    }

    // lua->script("draw()");
    ((*lua)["draw"])();
}

void tcApp::keyPressed(int key) {
    if(isFirstKey){
        std::string drawKeySource = R"LUA(
            function keyPressed(key)
                local c = string.char(key)
                -- print("keyPressed: " .. c)
                if c == "j" or c == "J" then
                    testJson()
                elseif c == "x" or c == "X" then
                    testXml()
                end
            end
        )LUA";

        lua->script(drawKeySource);

        isFirstKey = false;
    }

    // lua->script("keyPressed(" + std::to_string(key) + ")");
    ((*lua)["keyPressed"])(key);
}

void tcApp::addMessage(const std::string& msg){
    ((*lua)["addMessage"])(msg);
}

void tcApp::testJson() {
    addMessage("--- JSON Test ---");

    // Create JSON
    Json j;
    j["name"] = "TrussC";
    j["version"] = 0.1;
    j["features"] = {"graphics", "audio", "events"};
    j["settings"]["width"] = 1024;
    j["settings"]["height"] = 768;
    j["settings"]["fullscreen"] = false;

    // Convert to string
    string jsonStr = toJsonString(j, 2);
    addMessage("Created JSON:");

    // Display each line
    stringstream ss(jsonStr);
    string line;
    while (getline(ss, line)) {
        addMessage("  " + line);
    }

    // Save to file
    string path = "/tmp/trussc_test.json";
    if (saveJson(j, path)) {
        addMessage("Saved to: " + path);
    }

    // Load from file
    Json loaded = loadJson(path);
    if (!loaded.empty()) {
        addMessage("Loaded back:");
        addMessage("  name: " + loaded["name"].get<string>());
        addMessage("  version: " + to_string(loaded["version"].get<double>()));
        addMessage("  features count: " + to_string(loaded["features"].size()));
    }

    addMessage("");
}

void tcApp::testXml() {
    addMessage("--- XML Test ---");

    // Create XML
    Xml xml;
    xml.addDeclaration();

    auto root = xml.addRoot("project");
    root.append_attribute("name") = "TrussC";

    auto info = root.append_child("info");
    info.append_child("version").text() = "0.1";
    info.append_child("author").text() = "TrussC Team";

    auto features = root.append_child("features");
    features.append_child("feature").text() = "graphics";
    features.append_child("feature").text() = "audio";
    features.append_child("feature").text() = "events";

    // Convert to string
    string xmlStr = xml.toString();
    addMessage("Created XML:");

    // Display each line
    stringstream ss(xmlStr);
    string line;
    while (getline(ss, line)) {
        addMessage("  " + line);
    }

    // Save to file
    string path = "/tmp/trussc_test.xml";
    if (xml.save(path)) {
        addMessage("Saved to: " + path);
    }

    // Load from file
    Xml loaded = loadXml(path);
    if (!loaded.empty()) {
        addMessage("Loaded back:");
        auto loadedRoot = loaded.root();
        addMessage("  project name: " + string(loadedRoot.attribute("name").value()));
        addMessage("  version: " + string(loadedRoot.child("info").child("version").text().get()));

        int featureCount = 0;
        for (auto f : loadedRoot.child("features").children("feature")) {
            featureCount++;
        }
        addMessage("  features count: " + to_string(featureCount));
    }

    addMessage("");
}

void tcApp::keyReleased(int key) {}

void tcApp::mousePressed(Vec2 pos, int button) {}
void tcApp::mouseReleased(Vec2 pos, int button) {}
void tcApp::mouseMoved(Vec2 pos) {}
void tcApp::mouseDragged(Vec2 pos, int button) {}
void tcApp::mouseScrolled(Vec2 delta) {}

void tcApp::windowResized(int width, int height) {}
void tcApp::filesDropped(const vector<string>& files) {}
void tcApp::exit() {}
