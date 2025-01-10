#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

const char* ssid = "HOME WIFI";
const char* password = "WIFI PASSWORD";
//Days of the week
const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
// Default list of recipes
std::vector<String> recipes = {"Chicken Pasta", "Tacos", "Pizza", "Beef burger", "Chicken burger", "Carbonara", "Kebab", "Butter chicken curry", "Curry", "Nachos"};
// Web server
AsyncWebServer server(80);
// Generate a random table
String generateTable() {
  String table = "<table style='border-collapse: collapse; width: 100%; table-layout: fixed;'>";
  table += "<thead><tr style='background-color: #f2f2f2;'>";
  table += "<th style='border: 1px solid #ddd; padding: 8px; width: 50%;'>Day</th>";
  table += "<th style='border: 1px solid #ddd; padding: 8px; width: 50%;'>Recipe</th>";
  table += "</tr></thead><tbody>";
  // Create a copy of the recipes to shuffle
  std::vector<String> shuffledRecipes = recipes;
  // Shuffle the recipes
  for (size_t i = shuffledRecipes.size() - 1; i > 0; i--) {
    size_t j = random(0, i + 1); 
    std::swap(shuffledRecipes[i], shuffledRecipes[j]); 
  }

  for (int i = 0; i < 7; i++) {
    table += "<tr>";
    table += "<td style='border: 1px solid #ddd; padding: 8px; text-align: left;'>" + String(days[i]) + "</td>";
    table += "<td style='border: 1px solid #ddd; padding: 8px; text-align: left;'>" + shuffledRecipes[i % shuffledRecipes.size()] + "</td>";
    table += "</tr>";
  }

  table += "</tbody></table>";
  return table;
}

void handleAddRecipe(AsyncWebServerRequest* request) {
  if (request->hasParam("newRecipe")) {
    String newRecipe = request->getParam("newRecipe")->value();
    if (newRecipe.length() > 0) {
      recipes.push_back(newRecipe); 
    }
  }
  request->redirect("/"); 
}

void displayAll(AsyncWebServerRequest* request) {
  String response = "<!DOCTYPE html><html><head><title>Recipe List</title></head><body>";
  response += "<h1>All Recipes</h1>";
  response += "<ul>";

  for (const auto& recipe : recipes) {
    response += "<li>" + recipe + "</li>";
  }

  response += "</ul>";
  response += "<a href='/'>Back to Home</a>";
  response += "</body></html>";
  request->send(200, "text/html", response);
}

const int buttonPin = 4; // Button connected to pin 4

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Failed!");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Serve the main webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    String html = "<!DOCTYPE html><html><head><title>Weekly Dinner Plan</title></head><body>";
    html += "<h1>Weekly Dinner Plan</h1>";
    html += generateTable();
    html += "<h2>Add a New Recipe</h2>";
    html += "<form action='/add' method='POST'>";
    html += "Recipe: <input type='text' name='newRecipe'><br><br>";
    html += "<input type='submit' value='Add Recipe'>";
    html += "</form>";
    html += "<hr width='20%' size='2' color='black' align='left' noshade>";
    html += "<h3>Click here to see all recipes</h3>";
    html += "<form action='/' method='GET'>";
    html += "<input type='hidden' name='show' value='recipes'>";
    html += "<input type='submit' value='View All'>";
    html += "</form>";
    html += "<hr width='20%' size='2' color='black' align='left' noshade>";

    if (request->hasParam("show") && request->getParam("show")->value() == "recipes") {
        html += "<h2>All Recipes</h2>";
        html += "<ul>";
        for (const auto& recipe : recipes) {
            html += "<li>" + recipe + "</li>";
        }
        html += "</ul>";
    }

    html += "<p>Press button for a new weekly plan</p>";
    html += "<script>";
    html += "setInterval(() => {";
    html += "fetch('/button').then(res => res.text()).then(state => {";
    html += "if (state === 'pressed') window.location.reload();";
    html += "});";
    html += "}, 500);";
    html += "</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
});
  server.on("/button", HTTP_GET, [](AsyncWebServerRequest* request) {
    String state = (digitalRead(buttonPin) == LOW) ? "pressed" : "not pressed";
    request->send(200, "text/plain", state);
});

  server.on("/add", HTTP_POST, handleAddRecipe);
  server.on("/recipes", HTTP_GET, displayAll);
  server.begin();
}

void loop() {

}
