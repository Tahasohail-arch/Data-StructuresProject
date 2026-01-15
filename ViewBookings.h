#ifndef VIEW_BOOKINGS_H
#define VIEW_BOOKINGS_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <cmath>
#include "booking.h"

using namespace sf;

extern 
SoundBuffer clickSoundBuffer;
extern Sound clickSound;
extern SoundBuffer transitionSoundBuffer;
extern Sound transitionSound;

using namespace std;


void viewBookings(Font &font)
{
    RenderWindow window(VideoMode(1920, 1080), "View Bookings - OceanRoute Navigator");
    window.setFramerateLimit(60);
    
    
    View fixedView(FloatRect(0, 0, 1920, 1080));
    window.setView(fixedView);

    Texture bgTexture;
    if (!bgTexture.loadFromFile("mainmenu.jpg"))
    {
        cout << "Error loading background!" << endl;
    }
    Sprite bgSprite(bgTexture);
    bgSprite.setScale(1920.0f / bgTexture.getSize().x, 1080.0f / bgTexture.getSize().y);
    
    bgSprite.setColor(Color(100, 60, 70, 255));

    Clock animClock;
    
    
    float scrollOffset = 0.0f;
    float targetScrollOffset = 0.0f;
    float scrollSpeed = 80.0f;
    int totalBookings = 0;
    
    
    Booking *counter = bookingManager.head;
    while (counter != nullptr)
    {
        totalBookings++;
        counter = counter->next;
    }
    
    
    float bookingHeight = 100.0f;
    float visibleHeight = 680.0f; 
    float contentHeight = totalBookings * bookingHeight;
    float maxScroll = max(0.0f, contentHeight - visibleHeight);

    while (window.isOpen())
    {
        float time = animClock.getElapsedTime().asSeconds();
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
            {
                clickSound.play();
                window.close();
            }
            
            
            if (event.type == Event::MouseWheelScrolled)
            {
                targetScrollOffset -= event.mouseWheelScroll.delta * scrollSpeed;
                targetScrollOffset = max(0.0f, min(targetScrollOffset, maxScroll));
            }
            
            
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W)
                {
                    targetScrollOffset -= scrollSpeed;
                    targetScrollOffset = max(0.0f, targetScrollOffset);
                }
                if (event.key.code == Keyboard::Down || event.key.code == Keyboard::S)
                {
                    targetScrollOffset += scrollSpeed;
                    targetScrollOffset = min(targetScrollOffset, maxScroll);
                }
                if (event.key.code == Keyboard::PageUp)
                {
                    targetScrollOffset -= visibleHeight;
                    targetScrollOffset = max(0.0f, targetScrollOffset);
                }
                if (event.key.code == Keyboard::PageDown)
                {
                    targetScrollOffset += visibleHeight;
                    targetScrollOffset = min(targetScrollOffset, maxScroll);
                }
                if (event.key.code == Keyboard::Home)
                {
                    targetScrollOffset = 0.0f;
                }
                if (event.key.code == Keyboard::End)
                {
                    targetScrollOffset = maxScroll;
                }
            }
        }
        
        
        scrollOffset += (targetScrollOffset - scrollOffset) * 0.15f;

        window.clear();
        window.draw(bgSprite);

        
        RectangleShape overlay(Vector2f(1920, 1080));
        overlay.setFillColor(Color(15, 5, 8, 200));
        window.draw(overlay);

        
        RectangleShape topVignette(Vector2f(1920, 100));
        topVignette.setFillColor(Color(150, 20, 20, 60));
        topVignette.setPosition(0, 0);
        window.draw(topVignette);
        
        RectangleShape bottomVignette(Vector2f(1920, 100));
        bottomVignette.setFillColor(Color(150, 20, 20, 60));
        bottomVignette.setPosition(0, 980);
        window.draw(bottomVignette);

        
        for (int p = 0; p < 40; p++)
        {
            float px = static_cast<float>((p * 53) % 1920);
            float py = static_cast<float>(fmod(1080.0 - fmod(time * 12.0 + p * 35.0, 1150.0), 1150.0));
            float wobble = static_cast<float>(sin(time * 1.5 + p)) * 8.0f;
            CircleShape particle(2.0f);
            particle.setFillColor(Color(255, 80, 80, static_cast<Uint8>(30 + 25 * sin(time + p))));
            particle.setPosition(px + wobble, py);
            window.draw(particle);
        }

        
        RectangleShape mainGlow(Vector2f(1460, 940));
        mainGlow.setFillColor(Color(200, 40, 40, 30));
        mainGlow.setPosition(230, 60);
        window.draw(mainGlow);

        RectangleShape mainBox(Vector2f(1440, 920));
        mainBox.setFillColor(Color(15, 8, 12, 250));
        mainBox.setOutlineThickness(4);
        mainBox.setOutlineColor(Color(200, 50, 50));
        mainBox.setPosition(240, 70);
        window.draw(mainBox);
        
        
        RectangleShape innerBorder(Vector2f(1430, 910));
        innerBorder.setFillColor(Color::Transparent);
        innerBorder.setOutlineThickness(1);
        innerBorder.setOutlineColor(Color(255, 80, 80, 80));
        innerBorder.setPosition(245, 75);
        window.draw(innerBorder);

        
        RectangleShape topAccent(Vector2f(1440, 5));
        float accentPulse = static_cast<float>(sin(time * 3.0));
        topAccent.setFillColor(Color(static_cast<Uint8>(200 + 55 * accentPulse), 40, 40));
        topAccent.setPosition(240, 70);
        window.draw(topAccent);

        
        for (int c = 0; c < 4; c++)
        {
            float cx = (c % 2 == 0) ? 240 : 1680 - 60;
            float cy = (c < 2) ? 70 : 990 - 60;
            RectangleShape cornerH(Vector2f(70, 4));
            RectangleShape cornerV(Vector2f(4, 70));
            Uint8 cornerAlpha = static_cast<Uint8>(150 + 100 * sin(time * 3.0 + c * 0.8));
            cornerH.setFillColor(Color(255, 60, 60, cornerAlpha));
            cornerV.setFillColor(Color(255, 60, 60, cornerAlpha));
            cornerH.setPosition(cx, cy);
            cornerV.setPosition(cx, cy);
            window.draw(cornerH);
            window.draw(cornerV);
        }

        
        Text title;
        title.setFont(font);
        title.setString("BOOKING HISTORY");
        title.setCharacterSize(48);
        title.setStyle(Text::Bold);
        title.setLetterSpacing(2.0f);
        float titleGlow = 0.7f + 0.3f * static_cast<float>(sin(time * 2.0));
        title.setFillColor(Color(255, static_cast<Uint8>(50 + 30 * titleGlow), static_cast<Uint8>(50 + 30 * titleGlow)));
        title.setOutlineThickness(4);
        title.setOutlineColor(Color(100, 0, 0));
        FloatRect titleBounds = title.getLocalBounds();
        title.setPosition(960 - titleBounds.width / 2, 90);
        window.draw(title);
        
        
        RectangleShape titleUnderline(Vector2f(titleBounds.width + 60, 3));
        titleUnderline.setFillColor(Color(255, 60, 60, 180));
        titleUnderline.setPosition(960 - titleBounds.width / 2 - 30, 155);
        window.draw(titleUnderline);

        
        Text subtitle;
        subtitle.setFont(font);
        subtitle.setString("Your Cargo Shipment Records");
        subtitle.setCharacterSize(18);
        subtitle.setFillColor(Color(255, 150, 150));
        subtitle.setLetterSpacing(1.2f);
        FloatRect subBounds = subtitle.getLocalBounds();
        subtitle.setPosition(960 - subBounds.width / 2, 165);
        window.draw(subtitle);

        
        RectangleShape divider(Vector2f(900, 2));
        divider.setFillColor(Color(200, 60, 60, 100));
        divider.setPosition(510, 195);
        window.draw(divider);

        if (bookingManager.head == nullptr)
        {
            
            CircleShape emptyIconGlow(60);
            emptyIconGlow.setFillColor(Color(200, 40, 40, 40));
            emptyIconGlow.setOrigin(60, 60);
            emptyIconGlow.setPosition(960, 450);
            window.draw(emptyIconGlow);
            
            CircleShape emptyIcon(50);
            emptyIcon.setFillColor(Color(40, 20, 25, 200));
            emptyIcon.setOutlineThickness(4);
            emptyIcon.setOutlineColor(Color(200, 60, 60));
            emptyIcon.setOrigin(50, 50);
            emptyIcon.setPosition(960, 450);
            window.draw(emptyIcon);

            Text emptyMark;
            emptyMark.setFont(font);
            emptyMark.setString("?");
            emptyMark.setCharacterSize(50);
            emptyMark.setFillColor(Color(255, 100, 100));
            emptyMark.setPosition(945, 420);
            window.draw(emptyMark);

            Text noBookings;
            noBookings.setFont(font);
            noBookings.setString("No Bookings Found");
            noBookings.setCharacterSize(32);
            noBookings.setStyle(Text::Bold);
            noBookings.setFillColor(Color(255, 120, 120));
            FloatRect noBounds = noBookings.getLocalBounds();
            noBookings.setPosition(960 - noBounds.width / 2, 530);
            window.draw(noBookings);

            Text hint;
            hint.setFont(font);
            hint.setString("Book your first cargo route from the main menu!");
            hint.setCharacterSize(16);
            hint.setFillColor(Color(200, 120, 120));
            FloatRect hintBounds = hint.getLocalBounds();
            hint.setPosition(960 - hintBounds.width / 2, 580);
            window.draw(hint);
        }
        else
        {
            
            float headerY = 210;
            
            Text headerID("ID", font, 16);
            headerID.setFillColor(Color(255, 100, 100));
            headerID.setStyle(Text::Bold);
            headerID.setLetterSpacing(1.2f);
            headerID.setPosition(320, headerY);
            window.draw(headerID);

            Text headerCustomer("CUSTOMER", font, 16);
            headerCustomer.setFillColor(Color(255, 100, 100));
            headerCustomer.setStyle(Text::Bold);
            headerCustomer.setLetterSpacing(1.2f);
            headerCustomer.setPosition(480, headerY);
            window.draw(headerCustomer);

            Text headerRoute("ROUTE", font, 16);
            headerRoute.setFillColor(Color(255, 100, 100));
            headerRoute.setStyle(Text::Bold);
            headerRoute.setLetterSpacing(1.2f);
            headerRoute.setPosition(750, headerY);
            window.draw(headerRoute);

            Text headerDate("DATE", font, 16);
            headerDate.setFillColor(Color(255, 100, 100));
            headerDate.setStyle(Text::Bold);
            headerDate.setLetterSpacing(1.2f);
            headerDate.setPosition(1100, headerY);
            window.draw(headerDate);

            Text headerCost("COST", font, 16);
            headerCost.setFillColor(Color(255, 100, 100));
            headerCost.setStyle(Text::Bold);
            headerCost.setLetterSpacing(1.2f);
            headerCost.setPosition(1350, headerY);
            window.draw(headerCost);

            
            RectangleShape headerDiv(Vector2f(1300, 2));
            headerDiv.setFillColor(Color(200, 60, 60, 120));
            headerDiv.setPosition(300, 240);
            window.draw(headerDiv);

            
            float clipTop = 250;
            float clipBottom = 930;
            
            Booking *current = bookingManager.head;
            float baseY = 255;
            int count = 1;

            while (current != nullptr)
            {
                float yPos = baseY + (count - 1) * bookingHeight - scrollOffset;
                
                
                if (yPos + bookingHeight > clipTop && yPos < clipBottom)
                {
                    bool isHovered = FloatRect(290, yPos - 5, 1330, 95).contains(mousePos) && yPos > clipTop && yPos + 90 < clipBottom;
                    float animOffset = static_cast<float>(sin(time * 2.0 + count * 0.5)) * 2.0f;

                    
                    if (isHovered)
                    {
                        RectangleShape cardGlow(Vector2f(1340, 100));
                        cardGlow.setFillColor(Color(200, 40, 40, 40));
                        cardGlow.setPosition(285, yPos - 8);
                        window.draw(cardGlow);
                    }

                    
                    RectangleShape bookingBox(Vector2f(1320, 90));
                    if (isHovered)
                    {
                        bookingBox.setFillColor(Color(50, 20, 25, 250));
                        bookingBox.setOutlineThickness(3);
                        bookingBox.setOutlineColor(Color(255, 80, 80));
                    }
                    else
                    {
                        bookingBox.setFillColor(Color(25, 12, 15, 240));
                        bookingBox.setOutlineThickness(2);
                        bookingBox.setOutlineColor(Color(150, 50, 50));
                    }
                    bookingBox.setPosition(290, yPos + (isHovered ? -3 : animOffset));
                    window.draw(bookingBox);

                    
                    RectangleShape accentBar(Vector2f(5, 90));
                    accentBar.setFillColor(Color(255, 60, 60));
                    accentBar.setPosition(290, yPos + (isHovered ? -3 : animOffset));
                    window.draw(accentBar);

                    
                    CircleShape numBadgeGlow(22);
                    numBadgeGlow.setFillColor(Color(200, 40, 40, 60));
                    numBadgeGlow.setPosition(303, yPos + 23 + (isHovered ? -3 : animOffset));
                    window.draw(numBadgeGlow);
                    
                    CircleShape numBadge(18);
                    numBadge.setFillColor(Color(180, 40, 40));
                    numBadge.setOutlineThickness(2);
                    numBadge.setOutlineColor(Color(255, 80, 80));
                    numBadge.setPosition(307, yPos + 27 + (isHovered ? -3 : animOffset));
                    window.draw(numBadge);

                    Text numText;
                    numText.setFont(font);
                    numText.setString(to_string(count));
                    numText.setCharacterSize(16);
                    numText.setStyle(Text::Bold);
                    numText.setFillColor(Color(255, 200, 200));
                    numText.setPosition(count < 10 ? 320 : 315, yPos + 30 + (isHovered ? -3 : animOffset));
                    window.draw(numText);

                    
                    Text idText;
                    idText.setFont(font);
                    idText.setString(current->bookingID);
                    idText.setCharacterSize(15);
                    idText.setFillColor(Color(255, 120, 120));
                    idText.setStyle(Text::Bold);
                    idText.setPosition(355, yPos + 15 + (isHovered ? -3 : animOffset));
                    window.draw(idText);

                    
                    Text customerText;
                    customerText.setFont(font);
                    customerText.setString(current->customerName);
                    customerText.setCharacterSize(16);
                    customerText.setFillColor(Color(255, 200, 200));
                    customerText.setPosition(480, yPos + 15 + (isHovered ? -3 : animOffset));
                    window.draw(customerText);

                    
                    Text routeText;
                    routeText.setFont(font);
                    routeText.setString(current->origin + "  ->  " + current->destination);
                    routeText.setCharacterSize(15);
                    routeText.setFillColor(Color(255, 180, 180));
                    routeText.setPosition(700, yPos + 15 + (isHovered ? -3 : animOffset));
                    window.draw(routeText);

                    
                    Text dateText;
                    dateText.setFont(font);
                    dateText.setString(current->departureDate);
                    dateText.setCharacterSize(15);
                    dateText.setFillColor(Color(255, 160, 160));
                    dateText.setPosition(1050, yPos + 15 + (isHovered ? -3 : animOffset));
                    window.draw(dateText);

                    
                    Text costText;
                    costText.setFont(font);
                    costText.setString("$" + to_string(current->totalCost));
                    costText.setCharacterSize(18);
                    costText.setFillColor(Color(255, 200, 100));
                    costText.setStyle(Text::Bold);
                    costText.setPosition(1320, yPos + 12 + (isHovered ? -3 : animOffset));
                    window.draw(costText);

                    
                    Text statusText;
                    statusText.setFont(font);
                    statusText.setString("CONFIRMED");
                    statusText.setCharacterSize(11);
                    statusText.setFillColor(Color(100, 255, 150));
                    statusText.setLetterSpacing(1.1f);
                    statusText.setPosition(355, yPos + 55 + (isHovered ? -3 : animOffset));
                    window.draw(statusText);

                    
                    string pathPreview = "";
                    for (int i = 0; i < current->routePath.getSize() && i < 4; i++)
                    {
                        pathPreview += current->routePath[i];
                        if (i < current->routePath.getSize() - 1 && i < 3)
                            pathPreview += " > ";
                    }
                    if (current->routePath.getSize() > 4)
                        pathPreview += "...";

                    Text pathText;
                    pathText.setFont(font);
                    pathText.setString(pathPreview);
                    pathText.setCharacterSize(12);
                    pathText.setFillColor(Color(180, 120, 120));
                    pathText.setPosition(700, yPos + 55 + (isHovered ? -3 : animOffset));
                    window.draw(pathText);
                }

                current = current->next;
                count++;
            }

            
            if (maxScroll > 0)
            {
                
                RectangleShape scrollTrack(Vector2f(8, 680));
                scrollTrack.setFillColor(Color(40, 20, 25, 150));
                scrollTrack.setPosition(1620, 250);
                window.draw(scrollTrack);
                
                
                float thumbHeight = max(50.0f, (visibleHeight / contentHeight) * 680.0f);
                float thumbY = 250 + (scrollOffset / maxScroll) * (680.0f - thumbHeight);
                
                RectangleShape scrollThumb(Vector2f(8, thumbHeight));
                scrollThumb.setFillColor(Color(200, 60, 60));
                scrollThumb.setPosition(1620, thumbY);
                window.draw(scrollThumb);
                
                
                if (scrollOffset > 0)
                {
                    
                    Text upArrow;
                    upArrow.setFont(font);
                    upArrow.setString("^");
                    upArrow.setCharacterSize(20);
                    upArrow.setFillColor(Color(255, 100, 100, static_cast<Uint8>(150 + 100 * sin(time * 4))));
                    upArrow.setPosition(1616, 225);
                    window.draw(upArrow);
                }
                
                if (scrollOffset < maxScroll)
                {
                    
                    Text downArrow;
                    downArrow.setFont(font);
                    downArrow.setString("v");
                    downArrow.setCharacterSize(20);
                    downArrow.setFillColor(Color(255, 100, 100, static_cast<Uint8>(150 + 100 * sin(time * 4))));
                    downArrow.setPosition(1618, 935);
                    window.draw(downArrow);
                }
            }

            
            Text countInfo;
            countInfo.setFont(font);
            countInfo.setString("Total: " + to_string(totalBookings) + " booking" + (totalBookings != 1 ? "s" : ""));
            countInfo.setCharacterSize(14);
            countInfo.setFillColor(Color(200, 120, 120));
            countInfo.setPosition(300, 955);
            window.draw(countInfo);
            
            
            if (maxScroll > 0)
            {
                Text scrollHint;
                scrollHint.setFont(font);
                scrollHint.setString("Scroll: Mouse Wheel | Arrow Keys | Page Up/Down");
                scrollHint.setCharacterSize(12);
                scrollHint.setFillColor(Color(180, 100, 100));
                scrollHint.setPosition(550, 958);
                window.draw(scrollHint);
            }
        }

        
        bool backHover = FloatRect(50, 950, 180, 55).contains(mousePos);

        if (backHover)
        {
            RectangleShape backGlow(Vector2f(200, 75));
            backGlow.setFillColor(Color(200, 40, 40, 60));
            backGlow.setPosition(40, 940);
            window.draw(backGlow);
        }

        RectangleShape backButton(Vector2f(180, 55));
        backButton.setFillColor(backHover ? Color(180, 40, 40) : Color(120, 30, 30));
        backButton.setOutlineThickness(3);
        backButton.setOutlineColor(backHover ? Color(255, 100, 100) : Color(200, 60, 60));
        backButton.setPosition(50, 950);
        window.draw(backButton);

        Text backBtnText("BACK", font, 20);
        backBtnText.setStyle(Text::Bold);
        backBtnText.setFillColor(backHover ? Color(255, 200, 200) : Color(255, 150, 150));
        backBtnText.setLetterSpacing(1.5f);
        backBtnText.setPosition(105, 963);
        window.draw(backBtnText);

        static bool backClicked = false;
        if (Mouse::isButtonPressed(Mouse::Button::Left))
        {
            if (!backClicked && backHover)
            {
                backClicked = true;
                clickSound.play();
                transitionSound.play();
                window.close();
            }
        }
        else
        {
            backClicked = false;
        }

        
        Text instruction;
        instruction.setFont(font);
        instruction.setString("Press ESC or click BACK to return to menu");
        instruction.setCharacterSize(14);
        instruction.setFillColor(Color(180, 100, 100));
        FloatRect instrBounds = instruction.getLocalBounds();
        instruction.setPosition(960 - instrBounds.width / 2, 1020);
        window.draw(instruction);

        window.display();
    }
}

#endif 
