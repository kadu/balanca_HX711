#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include "Recipe.h"

class RecipeManager {
private:
    const char* RECIPES_FILE = "/recipes.json";
    std::vector<Recipe> recipes;

public:
    RecipeManager() {}

    bool begin() {
        if (!LittleFS.begin()) {
            Serial.println("LittleFS Mount Failed");
            return false;
        }
        loadRecipes();
        return true;
    }

    void loadRecipes() {
        recipes.clear();
        if (!LittleFS.exists(RECIPES_FILE)) return;

        File file = LittleFS.open(RECIPES_FILE, "r");
        if (!file) return;

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.print("JSON load error: ");
            Serial.println(error.c_str());
            return;
        }

        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject obj : arr) {
            Recipe r;
            r.name = obj["name"].as<String>();
            JsonArray stepsArr = obj["steps"];
            for (JsonObject stepObj : stepsArr) {
                RecipeStep step;
                step.type = (stepObj["type"] == "POUR") ? RecipeStepType::POUR : RecipeStepType::WAIT;
                step.amount = stepObj["amount"] | 0;
                step.time = stepObj["time"] | 0;
                r.steps.push_back(step);
            }
            recipes.push_back(r);
        }
    }

    bool saveRecipes() {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();

        for (const auto& r : recipes) {
            JsonObject obj = arr.add<JsonObject>();
            obj["name"] = r.name;
            JsonArray stepsArr = obj["steps"].to<JsonArray>();
            for (const auto& step : r.steps) {
                JsonObject stepObj = stepsArr.add<JsonObject>();
                stepObj["type"] = (step.type == RecipeStepType::POUR) ? "POUR" : "WAIT";
                stepObj["amount"] = step.amount;
                stepObj["time"] = step.time;
            }
        }

        File file = LittleFS.open(RECIPES_FILE, "w");
        if (!file) return false;
        
        serializeJson(doc, file);
        file.close();
        return true;
    }

    const std::vector<Recipe>& getRecipes() const { return recipes; }

    void addOrUpdateRecipe(const Recipe& newRecipe) {
        for (auto& r : recipes) {
            if (r.name == newRecipe.name) {
                r = newRecipe;
                saveRecipes();
                return;
            }
        }
        recipes.push_back(newRecipe);
        saveRecipes();
    }

    void deleteRecipe(String name) {
        for (auto it = recipes.begin(); it != recipes.end(); ++it) {
            if (it->name == name) {
                recipes.erase(it);
                saveRecipes();
                return;
            }
        }
    }

    String getRecipesJson() {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (const auto& r : recipes) {
            JsonObject obj = arr.add<JsonObject>();
            obj["name"] = r.name;
            JsonArray stepsArr = obj["steps"].to<JsonArray>();
            for (const auto& step : r.steps) {
                JsonObject stepObj = stepsArr.add<JsonObject>();
                stepObj["type"] = (step.type == RecipeStepType::POUR) ? "POUR" : "WAIT";
                stepObj["amount"] = step.amount;
                stepObj["time"] = step.time;
            }
        }
        String output;
        serializeJson(doc, output);
        return output;
    }
};

#endif
