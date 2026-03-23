#ifndef RECIPE_H
#define RECIPE_H

#include <Arduino.h>
#include <vector>

enum class RecipeStepType { POUR, WAIT };

struct RecipeStep {
    RecipeStepType type;
    int amount; // ml
    int time;   // seconds
};

struct Recipe {
    String name;
    std::vector<RecipeStep> steps;
};

#endif
