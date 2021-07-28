#include "../Headers/Application.hpp"
#include "../Headers/Menu.hpp"
#include "../Headers/LogicalParameter.hpp"
#include "../Headers/ChangedParametersQueue.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/DefaultFiles.hpp"
#include "../Headers/Menu.hpp"


const sf::Time Application::TimePerFrame = sf::seconds(1.f / Settings::mFramesPerSecond);

Application::Application(Menu &menu)
: mMenu(menu)
, mSettings(mMenu.getSettings())
{
    openWindow();
    loadAssets();

    std::unique_ptr<Button> buttons(new Button(mTextures, mFonts));
    mButtons = std::move(buttons);

    std::unique_ptr<Statistics> stats(new Statistics(mFonts));
    mStatistics = std::move(stats);

    std::unique_ptr<Background> bg(new Background(mTextures, mWindow));
    mBackground = std::move(bg);

    std::unique_ptr<KPSWindow> kpsWindow(new KPSWindow(mFonts));
    mKPSWindow = std::move(kpsWindow);

    mSettings.setWindowReference(mWindow);
}

void Application::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (mWindow.isOpen())
    {
        sf::Time dt = clock.restart();
        timeSinceLastUpdate += dt;
        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;

            processInput();
            update(TimePerFrame);
        }

        render();
    }
}

void Application::processInput()
{
    sf::Event event;
    mKeyPressingManager.readClickedKeys();
    while (mWindow.pollEvent(event))
    {
        if (mWindow.hasFocus())
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == Settings::KeyToClear)
                    {
                        mButtons->clear();
                        mStatistics->clear();
                        mCalculation.clear();
                        mKeyPressingManager.clear();
                    }

                    if (event.key.code == Settings::KeyExit)
                    {
                        mMenu.saveConfig();
                        mWindow.close();
                        return;
                    }
                }
            }

            mMenu.handleEvent(event);
            mKPSWindow->handleEvent(event);
            if (event.type == sf::Event::Closed)
            {
                mMenu.saveConfig();
                mWindow.close();
                return;
            }

            mSettings.handleEvent(event);
            mButtons->highlightKey(mSettings.getButtonToChangeIndex());

            if (mSettings.wasButtonAmountChanged())
            {
                handleEvent(event);
                mStatistics->resize();
                mButtons->resize();
                mKeyPressingManager.resize();
                mBackground->resize();
            }
        }
    }
    if (!Settings::WindowTitleBar)
        moveWindow();

    mKPSWindow->handleOwnEvent();
    mMenu.handleOwnEvent();
    ChangedParametersQueue &queue = mMenu.getChangedParametersQueue();
    while (!queue.isEmpty())
    {
        std::pair<const LogicalParameter::ID, std::shared_ptr<LogicalParameter>> pair = queue.pop();
        if (Statistics::parameterIdMatches(pair.first))
        {
            mStatistics->setupText();
        }
        if (Button::parameterIdMatches(pair.first))
        {
            mButtons->setupTextures();
            mButtons->setupKeyCounterTextVec();
            mButtons->highlightKey(mSettings.getButtonToChangeIndex());
        }
        if (KPSWindow::parameterIdMatches(pair.first))
        {
            mKPSWindow->setupText();
        }
        if (pair.first == LogicalParameter::ID::MainWndwTitleBar)
        {
            openWindow();
        }
        mWindow.setSize(sf::Vector2u(getWindowWidth(), getWindowHeight()));
        mWindow.setView(sf::View(sf::FloatRect(0, 0, mWindow.getSize().x, mWindow.getSize().y)));
        mBackground->setupTexture();
    }

    mCalculation.handleInput(mKeyPressingManager, Settings::Keys);
    mButtons->handleInput(mKeyPressingManager.mNeedToBeReleased, mKeyPressingManager);
}

void Application::update(sf::Time dt)
{
    mCalculation.update();
    mStatistics->update(mCalculation.getKeyPerSecond(), mCalculation.getBeatsPerMinute(), mKeyPressingManager.mClickedKeys);
    mButtons->update(mKeyPressingManager.mNeedToBeReleased);
    mMenu.update();
    mKPSWindow->update(mCalculation.getKeyPerSecond());
    mKeyPressingManager.clear();
    mSettings.update();
}

void Application::render()
{
    mWindow.clear();

    mWindow.draw(*mBackground);
    mWindow.draw(*mStatistics);
    mWindow.draw(*mButtons);
    mMenu.render();
    mKPSWindow->render();
    
    mWindow.display();
}

void Application::handleEvent(sf::Event event)
{
    mWindow.setSize(sf::Vector2u(getWindowWidth(), getWindowHeight()));

    sf::Vector2f windowSize(mWindow.getSize());
    sf::View view(sf::FloatRect(0, 0, windowSize.x, windowSize.y));
    mWindow.setView(view);
}

void Application::loadAssets()
{
    std::string defaultName = "Default";

    if (Settings::ButtonTexturePath == defaultName)
        mTextures.loadFromMemory(Textures::KeyButton, Settings::DefaultButtonTexture, 71200);
    else
        mTextures.loadFromFile(Textures::KeyButton, Settings::ButtonTexturePath);
    if (Settings::AnimationTexturePath == defaultName) 
        mTextures.loadFromMemory(Textures::ButtonAnimation, Settings::DefaultAnimationTexture, 100700);
    else
        mTextures.loadFromFile(Textures::ButtonAnimation, Settings::AnimationTexturePath);

    if (Settings::BackgroundTexturePath == defaultName)
        mTextures.loadFromMemory(Textures::Background, Settings::DefaultBackgroundTexture, 14600);
    else
        mTextures.loadFromFile(Textures::Background, Settings::BackgroundTexturePath);
    
    
    if (Settings::KeyCountersFontPath == defaultName)
        mFonts.loadFromMemory(Fonts::KeyCounters, Settings::KeyCountersDefaultFont, 446100);
    else
        mFonts.loadFromFile(Fonts::KeyCounters, Settings::KeyCountersFontPath);

    if (Settings::StatisticsFontPath == defaultName)
        mFonts.loadFromMemory(Fonts::Statistics, Settings::StatisticsDefaultFont, 446100);
    else
        mFonts.loadFromFile(Fonts::Statistics, Settings::StatisticsFontPath);

    if (Settings::KPSWindowTextFontPath == defaultName)
        mFonts.loadFromMemory(Fonts::KPSText, Settings::DefaultKPSWindowFont, 459300);
    else
        mFonts.loadFromFile(Fonts::KPSText, Settings::KPSWindowTextFontPath);

    if (Settings::KPSWindowNumberFontPath == defaultName)
        mFonts.loadFromMemory(Fonts::KPSNumber, Settings::DefaultKPSWindowFont, 459300);
    else
        mFonts.loadFromFile(Fonts::KPSNumber, Settings::KPSWindowNumberFontPath);

    sf::Image icon;
    icon.loadFromMemory(IconTexture, 53200);
    mWindow.setIcon(256, 256, icon.getPixelsPtr());
}

void Application::openWindow()
{
    mWindow.create(sf::VideoMode(getWindowWidth(), getWindowHeight()), 
        "JKPS", Settings::WindowTitleBar ? sf::Style::Default : sf::Style::None);
    mWindow.setKeyRepeatEnabled(false);
    mWindow.setFramerateLimit(60);
}

void Application::moveWindow()
{
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)
    &&  mWindow.hasFocus())
    {
        mWindow.setPosition(mWindow.getPosition() + 
            sf::Mouse::getPosition() - mLastMousePosition);
    }
    mLastMousePosition = sf::Mouse::getPosition();
}

unsigned Application::getWindowWidth()
{
    unsigned width =  
        Settings::ButtonTextureSize.x * Settings::ButtonAmount + 
        (int(Settings::ButtonAmount) - 1) * Settings::ButtonDistance + 
        Settings::WindowBonusSizeLeft + Settings::WindowBonusSizeRight;
    
    return width > 0 ? width : 1; 
}

unsigned Application::getWindowHeight()
{
    unsigned height = 
        Settings::ButtonTextureSize.y + Settings::WindowBonusSizeTop + 
        Settings::WindowBonusSizeBottom;
    
    return height > 0 ? height : 1;
}
