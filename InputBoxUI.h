#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <SFML/Graphics.hpp>
#include <string>
using namespace sf;
using namespace std;


class InputBox
{
public:
    RectangleShape box;     
    Text text;              
    string inputString;     
    bool isActive;          
    string label;           
    Text labelText;         

    
    InputBox(Font &font, string lbl, Vector2f position)
    {
        label = lbl;
        
        
        box.setSize(Vector2f(300, 40));
        box.setFillColor(Color(0, 20, 60, 240));        
        box.setOutlineThickness(2);
        box.setOutlineColor(Color(100, 200, 255));      
        box.setPosition(position);

        
        labelText.setFont(font);
        labelText.setString(label + ":");
        labelText.setCharacterSize(16);
        labelText.setFillColor(Color(200, 230, 255));
        labelText.setPosition(position.x, position.y - 25);  

        
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(Color::White);
        text.setPosition(position.x + 10, position.y + 8);   

        inputString = "";       
        isActive = false;       
    }

    
    void handleEvent(Event &event, RenderWindow &window)
    {
        
        if (event.type == Event::MouseButtonPressed)
        {
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            
            if (box.getGlobalBounds().contains(mousePos))
            {
                
                isActive = true;
                box.setOutlineColor(Color::Yellow);     
            }
            else
            {
                
                isActive = false;
                box.setOutlineColor(Color(100, 200, 255));  
            }
        }

        
        if (event.type == Event::TextEntered && isActive)
        {
            if (event.text.unicode == '\b')  
            {
                
                if (!inputString.empty())
                {
                    inputString.pop_back();
                }
            }
            else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t')
            {
                
                inputString += static_cast<char>(event.text.unicode);
            }
            
            
            text.setString(inputString);
        }
    }

    
    void draw(RenderWindow &window)
    {
        window.draw(labelText);     
        window.draw(box);            
        window.draw(text);           
    }

    
    string getText() 
    { 
        return inputString; 
    }
    
    
    void setText(string newText)
    {
        inputString = newText;
        text.setString(inputString);
    }
};

#endif
