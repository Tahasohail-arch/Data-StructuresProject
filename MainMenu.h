#ifndef MAINMENU_H
#define MAINMENU_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include "Vector.h"

using namespace sf;
using namespace std;

extern int menuSelection;
extern const int numLocations;
extern PortLocation portLocations[];


extern sf::SoundBuffer clickSoundBuffer;
extern sf::Sound clickSound;


extern sf::SoundBuffer transitionSoundBuffer;
extern sf::Sound transitionSound;


void drawMainMenu(RenderWindow &window, Font &font)
{
    float centerX = 960;
    Vector2i mousePos = Mouse::getPosition(window);
    Vector2f mouse(mousePos.x, mousePos.y);

    static Clock animClock;
    static Clock flickerClock;
    float time = animClock.getElapsedTime().asSeconds();

    for (int i = 0; i < 1080; i++)
    {
        RectangleShape line(Vector2f(1920, 1));
        line.setPosition(0, i);
        float ratio = i / 1080.0f;
        float pulse = 0.8f + 0.2f * sin(time * 0.5f);
        int r = static_cast<int>((10 + ratio * 25) * pulse);
        int g = static_cast<int>(0 + ratio * 3);
        int b = static_cast<int>((5 + ratio * 12) * pulse);
        line.setFillColor(Color(r, g, b));
        window.draw(line);
    }

    float lightsY = 180;
    for (int i = 0; i < 40; i++)
    {
        float x1 = i * 50.0f;
        float x2 = (i + 1) * 50.0f;
        float sag1 = sin(i * 0.3f) * 15.0f + 10.0f;
        float sag2 = sin((i + 1) * 0.3f) * 15.0f + 10.0f;
        
        Vertex wire[] = {
            Vertex(Vector2f(x1, lightsY + sag1), Color(40, 40, 40)),
            Vertex(Vector2f(x2, lightsY + sag2), Color(40, 40, 40))
        };
        window.draw(wire, 2, Lines);
    }
    
    Color lightColors[] = {Color(255, 50, 50), Color(50, 255, 50), Color(50, 50, 255), 
                           Color(255, 255, 50), Color(255, 100, 255), Color(50, 255, 255)};
    for (int i = 0; i < 38; i++)
    {
        float x = 25.0f + i * 50.0f;
        float sag = sin(i * 0.3f) * 15.0f + 10.0f;
        
        float flicker = 1.0f;
        int flickerRand = static_cast<int>(time * 20 + i * 13) % 100;
        if (flickerRand > 92) flicker = 0.2f;
        else if (flickerRand > 88) flicker = 0.5f;
        
        float pulse = 0.7f + 0.3f * sin(time * 3.0f + i * 0.5f);
        
        Color c = lightColors[i % 6];
        int alpha = static_cast<int>(255 * flicker * pulse);
        
        CircleShape glow(12);
        glow.setOrigin(12, 12);
        glow.setPosition(x, lightsY + sag + 15);
        glow.setFillColor(Color(c.r, c.g, c.b, static_cast<int>(60 * flicker * pulse)));
        window.draw(glow);
        
        CircleShape bulb(6);
        bulb.setOrigin(6, 6);
        bulb.setPosition(x, lightsY + sag + 15);
        bulb.setFillColor(Color(c.r, c.g, c.b, alpha));
        window.draw(bulb);
        
        CircleShape bright(2);
        bright.setOrigin(2, 2);
        bright.setPosition(x, lightsY + sag + 14);
        bright.setFillColor(Color(255, 255, 255, static_cast<int>(200 * flicker * pulse)));
        window.draw(bright);
    }

    for (int i = 0; i < 80; i++)
    {
        float baseX = static_cast<float>((i * 73) % 1920);
        float speed = 12.0f + (i % 10) * 4.0f;
        float particleY = static_cast<float>(fmod(1100.0 - fmod(time * speed + i * 45.0, 1200.0), 1200.0));
        float wobble = static_cast<float>(sin(time * 2.0 + i * 0.5)) * 20.0f;
        float size = 1.0f + (i % 5) * 0.6f;
        
        
        float flicker = 0.4f + 0.6f * static_cast<float>(sin(time * 6 + i * 0.7));
        CircleShape spore(size);
        spore.setPosition(baseX + wobble, particleY);
        int alpha = static_cast<int>(50 + flicker * 100);
        
        if (i % 3 == 0)
            spore.setFillColor(Color(200, 220, 255, alpha)); 
        else
            spore.setFillColor(Color(255, 200, 150, alpha / 2)); 
        window.draw(spore);
    }

    for (int side = 0; side < 2; side++)
    {
        for (int i = 0; i < 8; i++)
        {
            float baseX = (side == 0) ? 0 : 1920;
            float yPos = 200.0f + i * 100.0f;
            float length = 80.0f + sin(time * 0.8f + i) * 40.0f;
            float waveY = sin(time * 1.5f + i * 0.7f) * 20.0f;
            
            for (int seg = 0; seg < 5; seg++)
            {
                float segLen = length * (1.0f - seg * 0.15f);
                float segX = (side == 0) ? segLen * (seg * 0.2f) : 1920 - segLen * (seg * 0.2f);
                float thickness = 8.0f - seg * 1.2f;
                
                RectangleShape vine(Vector2f(segLen / 5, thickness));
                vine.setPosition((side == 0) ? seg * (length / 5) : 1920 - seg * (length / 5) - segLen / 5, 
                                yPos + waveY + seg * 5);
                vine.setFillColor(Color(60 - seg * 10, 20, 30, 150 - seg * 25));
                window.draw(vine);
            }
        }
    }

    float portalPulse = 0.5f + 0.5f * sin(time * 0.8f);
    for (int ring = 0; ring < 5; ring++)
    {
        CircleShape portal(150 - ring * 25);
        portal.setOrigin(150 - ring * 25, 150 - ring * 25);
        portal.setPosition(centerX, 520);
        portal.setFillColor(Color::Transparent);
        int portalAlpha = static_cast<int>((15 - ring * 3) * portalPulse);
        portal.setOutlineColor(Color(255, 30, 30, portalAlpha));
        portal.setOutlineThickness(3);
        window.draw(portal);
    }

    
    float vignetteIntensity = 0.85f + 0.12f * static_cast<float>(sin(time * 1.0));
    for (int ring = 0; ring < 10; ring++)
    {
        RectangleShape vignette(Vector2f(1920 + ring * 50, 1080 + ring * 50));
        vignette.setPosition(-ring * 25.0f, -ring * 25.0f);
        vignette.setFillColor(Color::Transparent);
        int alpha = static_cast<int>((ring * 10) * vignetteIntensity);
        vignette.setOutlineColor(Color(0, 0, 0, alpha));
        vignette.setOutlineThickness(35 + ring * 20);
        window.draw(vignette);
    }

    string titleStr = "OCEAN ROUTE";
    float titleY = 60;

    float globalFlicker = 1.0f;
    int flickerRand = static_cast<int>(time * 25) % 100;
    if (flickerRand > 97) globalFlicker = 0.2f;
    else if (flickerRand > 94) globalFlicker = 0.5f;

    float totalWidth = 0;
    for (size_t i = 0; i < titleStr.length(); i++)
    {
        Text measureText(string(1, titleStr[i]), font, 100);
        measureText.setStyle(Text::Bold);
        if (titleStr[i] == ' ')
            totalWidth += 40;
        else
            totalWidth += measureText.getLocalBounds().width + 15;
    }

    float titleStartX = centerX - totalWidth / 2;
    float currentX = titleStartX;

    
    for (int glowLayer = 5; glowLayer >= 0; glowLayer--)
    {
        float glowX = titleStartX;
        for (size_t i = 0; i < titleStr.length(); i++)
        {
            if (titleStr[i] == ' ')
            {
                glowX += 40;
                continue;
            }

            Text glowText(string(1, titleStr[i]), font, 100 + glowLayer * 3);
            glowText.setStyle(Text::Bold);
            
            
            float letterFlicker = 1.0f;
            int letterRand = static_cast<int>(time * 12 + i * 19) % 100;
            if (letterRand > 96) letterFlicker = 0.3f;

            int glowAlpha = static_cast<int>((50 - glowLayer * 8) * globalFlicker * letterFlicker);
            glowText.setFillColor(Color(255, 20, 20, glowAlpha));
            glowText.setPosition(glowX - glowLayer * 1.5f, titleY - glowLayer * 1.5f);
            window.draw(glowText);

            glowX += glowText.getLocalBounds().width + 15;
        }
    }

    
    currentX = titleStartX;
    for (size_t i = 0; i < titleStr.length(); i++)
    {
        if (titleStr[i] == ' ')
        {
            currentX += 40;
            continue;
        }

        Text charText(string(1, titleStr[i]), font, 100);
        charText.setStyle(Text::Bold);
        
        
        float charWave = static_cast<float>(sin(time * 1.8 + i * 0.3)) * 3;
        
        
        float letterFlicker = 1.0f;
        int letterRand = static_cast<int>(time * 12 + i * 19) % 100;
        if (letterRand > 96) letterFlicker = 0.4f;

        
        int red = static_cast<int>(255 * globalFlicker * letterFlicker);
        int green = static_cast<int>((40 + 25 * sin(time * 2.5 + i * 0.4)) * globalFlicker * letterFlicker);
        int blue = static_cast<int>(40 * globalFlicker * letterFlicker);
        
        charText.setFillColor(Color(red, green, blue));
        charText.setOutlineThickness(3);
        charText.setOutlineColor(Color(180, 0, 0, static_cast<int>(220 * globalFlicker * letterFlicker)));

        charText.setPosition(currentX, titleY + charWave);
        window.draw(charText);

        currentX += charText.getLocalBounds().width + 15;
    }

    
    Text navText("N A V I G A T O R", font, 36);
    navText.setStyle(Text::Bold);
    navText.setLetterSpacing(3.0f);
    float navFlicker = ((int)(time * 15) % 100 > 93) ? 0.4f : 1.0f;
    navText.setFillColor(Color(255, static_cast<int>(80 * navFlicker), static_cast<int>(80 * navFlicker), 
                               static_cast<int>(255 * globalFlicker * navFlicker)));
    navText.setOutlineThickness(1);
    navText.setOutlineColor(Color(150, 0, 0, static_cast<int>(150 * navFlicker)));
    FloatRect navBounds = navText.getLocalBounds();
    navText.setOrigin(navBounds.width / 2, 0);
    navText.setPosition(centerX, 165);
    window.draw(navText);

    auto makeButton = [&](string txt, float yPos, bool &hovered, int id)
    {
        FloatRect bounds(centerX - 260, yPos - 45, 520, 90);
        hovered = bounds.contains(mouse);

        float floatOffset = static_cast<float>(sin(time * 0.9 + id * 0.8)) * 2.5f;
        float yFinal = yPos + (hovered ? -4.0f : floatOffset);

        float btnFlicker = 1.0f;
        int btnRand = static_cast<int>(time * 22 + id * 17) % 100;
        if (hovered && btnRand > 93) btnFlicker = 0.5f;

        if (hovered)
        {
            for (int g = 4; g >= 0; g--)
            {
                RectangleShape outerGlow(Vector2f(520 + g * 25, 90 + g * 25));
                outerGlow.setPosition(centerX - 260 - g * 12.5f, yFinal - 45 - g * 12.5f);
                int glowAlpha = static_cast<int>((35 - g * 6) * btnFlicker);
                outerGlow.setFillColor(Color(255, 20, 0, glowAlpha));
                window.draw(outerGlow);
            }
        }

        RectangleShape card(Vector2f(520, 90));
        card.setPosition(centerX - 260, yFinal - 45);
        if (hovered)
            card.setFillColor(Color(50, 8, 8, static_cast<int>(245 * btnFlicker)));
        else
            card.setFillColor(Color(25, 3, 5, 210));
        
        card.setOutlineThickness(hovered ? 3.5f : 2.0f);
        int borderAlpha = static_cast<int>((hovered ? 255 : 140) * btnFlicker);
        int borderGreen = hovered ? static_cast<int>(40 + 25 * sin(time * 5)) : 0;
        card.setOutlineColor(Color(220, borderGreen, 0, borderAlpha));
        window.draw(card);

        float lineWidth = hovered ? 520.0f : 300.0f + static_cast<float>(sin(time * 0.7 + id)) * 120.0f;
        RectangleShape topLine(Vector2f(lineWidth, 2));
        topLine.setPosition(centerX - lineWidth / 2, yFinal - 45);
        int lineAlpha = static_cast<int>((hovered ? 255 : 180) * btnFlicker);
        topLine.setFillColor(Color(255, hovered ? 60 : 20, 0, lineAlpha));
        window.draw(topLine);
        
        if (hovered)
        {
            RectangleShape bottomLine(Vector2f(lineWidth * 0.6f, 2));
            bottomLine.setPosition(centerX - lineWidth * 0.3f, yFinal + 43);
            bottomLine.setFillColor(Color(255, 40, 0, static_cast<int>(150 * btnFlicker)));
            window.draw(bottomLine);
        }

        
        Color indicatorColor = lightColors[id % 6];
        float indicatorPulse = 0.6f + 0.4f * sin(time * 4.0f + id);
        
        CircleShape indicator(hovered ? 10 : 7);
        indicator.setOrigin(hovered ? 10 : 7, hovered ? 10 : 7);
        indicator.setPosition(centerX - 230, yFinal);
        
        if (hovered)
        {
            
            CircleShape indGlow(18);
            indGlow.setOrigin(18, 18);
            indGlow.setPosition(centerX - 230, yFinal);
            indGlow.setFillColor(Color(indicatorColor.r, indicatorColor.g, indicatorColor.b, 
                                      static_cast<int>(80 * indicatorPulse * btnFlicker)));
            window.draw(indGlow);
        }
        
        indicator.setFillColor(Color(indicatorColor.r, indicatorColor.g, indicatorColor.b, 
                                    static_cast<int>(255 * (hovered ? 1.0f : 0.5f) * indicatorPulse * btnFlicker)));
        window.draw(indicator);

        
        Text label(txt, font, hovered ? 38 : 35);
        label.setStyle(Text::Bold);
        
        int textRed = static_cast<int>(255 * btnFlicker);
        int textGreen = hovered ? static_cast<int>(220 + 25 * sin(time * 3)) : 200;
        label.setFillColor(Color(textRed, textGreen, textGreen, static_cast<int>(255 * btnFlicker)));
        label.setOutlineThickness(hovered ? 2 : 1);
        label.setOutlineColor(Color(180, 0, 0, static_cast<int>(200 * btnFlicker)));
        
        FloatRect lb = label.getLocalBounds();
        label.setOrigin(lb.width / 2, lb.height / 2);
        label.setPosition(centerX + 15, yFinal - 2);
        window.draw(label);
    };

    bool hoverMap = false, hoverBook = false, hoverExit = false, hoverView = false, hoverMultiLeg = false;

    makeButton("Show World Map", 280, hoverMap, 1);
    makeButton("Book Cargo", 380, hoverBook, 2);
    makeButton("View Bookings", 480, hoverView, 3);
    makeButton("Multi-Leg Route", 580, hoverMultiLeg, 4);
    makeButton("Exit", 680, hoverExit, 5);

    float lineY = 880;
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (size_t i = 0; i < alphabet.length(); i++)
    {
        float letterX = 180 + i * 62;
        float letterWave = sin(time * 1.2f + i * 0.3f) * 3.0f;
        
        
        float letterFlicker = 1.0f;
        int lRand = static_cast<int>(time * 18 + i * 11) % 100;
        if (lRand > 95) letterFlicker = 0.2f;
        else if (lRand > 90) letterFlicker = 0.6f;
        
        
        Text letter(string(1, alphabet[i]), font, 28);
        letter.setStyle(Text::Bold);
        
        
        Color lColor = lightColors[i % 6];
        int lAlpha = static_cast<int>(180 * letterFlicker);
        letter.setFillColor(Color(lColor.r, lColor.g, lColor.b, lAlpha));
        letter.setPosition(letterX, lineY + letterWave);
        window.draw(letter);
    }

    
    Text subtitle("T H E   U P S I D E   D O W N", font, 16);
    subtitle.setStyle(Text::Bold);
    float subFlicker = ((int)(time * 10) % 100 > 94) ? 0.3f : 1.0f;
    subtitle.setFillColor(Color(200, 60, 60, static_cast<int>(140 * subFlicker)));
    FloatRect sb = subtitle.getLocalBounds();
    subtitle.setOrigin(sb.width / 2, sb.height / 2);
    subtitle.setPosition(centerX, 950);
    window.draw(subtitle);
    
    
    Text credits("Press any button to continue...", font, 14);
    float credFlicker = 0.7f + 0.3f * sin(time * 2.0f);
    credits.setFillColor(Color(150, 100, 100, static_cast<int>(120 * credFlicker)));
    FloatRect cb = credits.getLocalBounds();
    credits.setOrigin(cb.width / 2, cb.height / 2);
    credits.setPosition(centerX, 1000);
    window.draw(credits);

    
    static bool clicked = false;
    if (Mouse::isButtonPressed(Mouse::Button::Left))
    {
        if (!clicked)
        {
            if (hoverMap)
            {
                clickSound.play();
                transitionSound.play();
                menuSelection = 1;
            }
            if (hoverBook)
            {
                clickSound.play();
                transitionSound.play();
                menuSelection = 2;
            }
            if (hoverView)
            {
                clickSound.play();
                transitionSound.play();
                menuSelection = 3;
            }
            if (hoverMultiLeg)
            {
                clickSound.play();
                transitionSound.play();
                menuSelection = 4;
            }
            if (hoverExit)
            {
                clickSound.play();
                menuSelection = 5;
            }
            clicked = true;
        }
    }
    else
            clicked = false;
}


int openMainMenu(Font &font)
{
    RenderWindow window(VideoMode(1920, 1080), "Main Menu - OceanRoute Navigator");
    
    
    View fixedView(FloatRect(0, 0, 1920, 1080));
    window.setView(fixedView);

    Music menuMusic;
    if (!menuMusic.openFromFile("strangerthings_theme.mp3"))
        cout << "Error loading strangerthings_theme.mp3" << endl;
    else
    {
        menuMusic.setLoop(true);
        menuMusic.setVolume(50);
        menuMusic.play();
    }

    Texture bgTexture;
    if (!bgTexture.loadFromFile("mainmenu.jpg"))
        cout << "Error loading mainmenu.jpg " << endl;

    Sprite bgSprite;
    bgSprite.setTexture(bgTexture);

    bgSprite.setScale(
        1920.0f / bgTexture.getSize().x,
        1080.0f / bgTexture.getSize().y);

    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();
        }

        window.clear();

        window.draw(bgSprite);
        RectangleShape overlay(sf::Vector2f(1920, 1080));
        overlay.setFillColor(Color(0, 0, 0, 80));
        window.draw(overlay);

        drawMainMenu(window, font);

        window.display();

        if (menuSelection != 0)
        {
            int chosen = menuSelection;
            menuSelection = 0;
            menuMusic.stop(); 
            window.close();
            return chosen;
        }
    }
    menuMusic.stop(); 
    return 3;
}

#endif 
