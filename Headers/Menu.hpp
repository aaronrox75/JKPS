#pragma once

#include "ParameterLine.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "LogicalParameter.hpp"
#include "Settings.hpp"
#include "ChangedParametersQueue.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <vector>

class Button;


class Menu 
{
    public:
        Menu();

        void processInput();
        void update();
        void render();

        void openWindow();
        void closeWindow();
        bool isOpen() const;

        LogicalParameter &getParameter(LogicalParameter::ID id);
        ChangedParametersQueue &getChangedParametersQueue();

        void saveConfig(const std::vector<std::unique_ptr<Button>> &mKeys);
        void requestFocus();
        void requestReloadAssets();
        bool resetReloadAssetsRequest();


    private:
        void handleEvent();
        void handleRealtimeInput();

        void loadFonts();
        void loadTextures();

        void buildMenuTabs();
        void buildParametersMap();
        void buildParameterLines();

        void moveSliderBarButtons(float offset);
        void moveSliderBarMouse(sf::Vector2i mousePos);
        void returnViewInBounds();


    private:
        sf::RenderWindow mWindow;

        FontHolder mFonts;
        TextureHolder mTextures;

        sf::View mView;
        const float mScrollSpeed;
        float mHighViewBounds;

        sf::RectangleShape mSliderBar;
        const sf::Color mSliderBarDefaultColor;
        const sf::Color mSliderBarAimedColor;
        const sf::Color mSliderBarPressedColor;

        unsigned mCurrentTab;
        std::vector<std::unique_ptr<GfxParameter>> mTabs;
        std::vector<float> mBounds;
        sf::RectangleShape mTabsBackground;

        std::map<LogicalParameter::ID, std::shared_ptr<LogicalParameter>> mParameters;
        std::map<ParameterLine::ID, std::shared_ptr<ParameterLine>> mParameterLines;
        ChangedParametersQueue mChangedParametersQueue;

        static bool paramValWasChanged;
        static std::string mProgramVersion;
};