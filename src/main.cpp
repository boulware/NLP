#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>

#include <iostream>
#include <fstream>
#include <cstdint>
//#include <cmath>

#include <string>
#include <vector>
#include <unordered_map>

sf::Font CourierFont;

enum class grammatical_tense
{
    Present = 0,
    Past,
};

enum class grammatical_aspect
{
    Simple = 0,
    Continuous,
    Perfect,
};

enum class grammatical_person
{
    First,
    Second,
    Third,
};

enum class grammatical_number
{
    Singular,
    Plural,
};

enum class verb_transitivity
{
    intransitive,
    transitive,
    ditransitive
};

struct feature_structure
{
public:
    grammatical_number Number;// = grammatical_number::Singular;
    grammatical_person Person;// = grammatical_person::First;
    grammatical_tense Tense;// = grammatical_tense::Present;
    grammatical_aspect Aspect;// = grammatical_aspect::Simple;
    
    feature_structure() : Number(grammatical_number::Singular),
                                   Person(grammatical_person::First),
                                   Tense(grammatical_tense::Present),
                                   Aspect(grammatical_aspect::Simple) {}
    
    feature_structure(grammatical_number Number,
                               grammatical_person Person,
                               grammatical_tense Tense,
                               grammatical_aspect Aspect) : Number(Number),
                                                            Person(Person),
                                                            Tense(Tense),
                                                            Aspect(Aspect) {}
};

bool operator==(const feature_structure& left, const feature_structure& right)
{
    if(left.Number == right.Number &&
       left.Person == right.Person &&
       left.Tense == right.Tense &&
       left.Aspect == right.Aspect)
    {
        return true;
    }
    return false;
}

struct FeatureStructureKeyHasher
{
    std::size_t operator()(const feature_structure& k) const
    {
        using std::size_t;
        using std::hash;
        using std::string;

        return (
            (hash<std::size_t>()(static_cast<std::size_t>(k.Number))) ^
            (hash<std::size_t>()(static_cast<std::size_t>(k.Person)) << 4) ^
            (hash<std::size_t>()(static_cast<std::size_t>(k.Tense)) << 8) ^
            (hash<std::size_t>()(static_cast<std::size_t>(k.Aspect)) << 12)
                );
    }
};

class inflection
{
public:
    uint8_t mTrimCount;
    std::string mSuffix;
public:
    inflection() : mTrimCount(0), mSuffix("") {}
    inflection(uint8_t TrimCount, std::string Suffix) : mTrimCount(TrimCount),  mSuffix(Suffix) {}

    std::string Inflect(const std::string& Lemma)
    {
        std::string InflectedString = Lemma;
        
        for(int i = 0; i < mTrimCount; i++)
        {
            InflectedString.pop_back();
        }

        InflectedString += mSuffix;

        return InflectedString;
    }
};

class inflection_table
{
private:
    std::unordered_map<feature_structure, inflection, FeatureStructureKeyHasher> Inflections;
public:
    inflection_table() {}
    inflection_table(std::string FilePath)
    {
        std::vector<inflection> FileInflections;
        
        std::ifstream File(FilePath);
        std::string Line;
    
        if(File.is_open())
        {
            int TrimCount = 0;
            std::string Suffix = "";
            
            char Character;
            
            while(File >> std::noskipws >> Character)
            {
                switch(Character)
                {
                    case('-'):
                    {
                        TrimCount++;
                    } break;
                    case('\n'):
                    case(','):
                    {
                        FileInflections.push_back(inflection(TrimCount, Suffix));
                        
                        TrimCount = 0;
                        Suffix = "";

                    } break;
                    case('.'): break;
                    default:
                    {
                        Suffix += Character;
                    } break;
                }
            }
        }
        
        for(int i = 0; i < FileInflections.size(); i++)
        {            
            switch(i)
            {
                case(0): Inflections[feature_structure(grammatical_number::Singular,
                                                                grammatical_person::First,
                                                                grammatical_tense::Present,
                                                                grammatical_aspect::Simple)] = FileInflections[i]; break;
                case(1): Inflections[feature_structure(grammatical_number::Singular,
                                                                grammatical_person::Second,
                                                                grammatical_tense::Present,
                                                                grammatical_aspect::Simple)] = FileInflections[i]; break;
                case(2): Inflections[feature_structure(grammatical_number::Singular,
                                                                grammatical_person::Third,
                                                                grammatical_tense::Present,
                                                                grammatical_aspect::Simple)] = FileInflections[i]; break;
                case(3): Inflections[feature_structure(grammatical_number::Plural,
                                                                grammatical_person::First,
                                                                grammatical_tense::Present,
                                                                grammatical_aspect::Simple)] = FileInflections[i]; break;
                case(4): Inflections[feature_structure(grammatical_number::Plural,
                                                                grammatical_person::Second,
                                                                grammatical_tense::Present,
                                                                grammatical_aspect::Simple)] = FileInflections[i]; break;
                case(5): Inflections[feature_structure(grammatical_number::Plural,
                                                                grammatical_person::Third,
                                                                grammatical_tense::Present,
                                                                grammatical_aspect::Simple)] = FileInflections[i]; break;
            }
        }
    }

    inflection QueryInflection(feature_structure Query)
    {
        return Inflections[Query];
    }
};

inflection_table NullInflectionTable;

enum class lexeme_type
{
    undefined = 0,
    noun,
    verb,
};

class interaction;

class lexeme_node;
class LexemeKeyHasher;

class grammar;

class lexeme
{
    friend class lexeme_node;
    friend class LexemeKeyHasher;
private:
    lexeme_type mType;
    std::string mLemma;
    inflection_table& mInflectionTable;

    std::vector<lexeme*> mInteractions;

public:
    lexeme(const lexeme& Source) : mLemma(Source.mLemma), mInflectionTable(Source.mInflectionTable) {}
    lexeme& operator=(const lexeme& Source)
    {
        mType = Source.mType;
        mLemma = Source.mLemma;
        mInflectionTable = Source.mInflectionTable;
    }
    lexeme(lexeme_type Type, std::string Lemma, inflection_table& InflectionTable) : mType(Type), mLemma(Lemma), mInflectionTable(InflectionTable) {}
    lexeme() : lexeme(lexeme_type::undefined, "", NullInflectionTable) {}

    std::string Inflect(feature_structure FS)
    {
        return mInflectionTable.QueryInflection(FS).Inflect(mLemma);
    }
};

lexeme NullLexeme(lexeme_type::undefined, "_null_", NullInflectionTable);

class lexeme_node : public sf::Drawable, public sf::Transformable
{
private:
    sf::CircleShape Outline;
    sf::Text Text;
public:
    float Scale;

    lexeme_node(lexeme Lexeme, float Scale) : Scale(Scale), Outline(Scale), Text(Lexeme.mLemma, CourierFont, 50.f)
    {
        Outline.setFillColor(sf::Color::Transparent);
        Outline.setOutlineThickness(-4.f);
        Outline.setOutlineColor(sf::Color::Green);
        Outline.setPosition(-Scale, -Scale);

        
        sf::FloatRect LocalTextRect = Text.getLocalBounds();
        sf::FloatRect GlobalTextRect = Text.getGlobalBounds();
        float GlobalWidth = LocalTextRect.width - LocalTextRect.left;
        float TextScale = 1.5f * Scale / GlobalWidth;
        
        Text.setScale(TextScale, TextScale);
        Text.setOrigin(LocalTextRect.left + LocalTextRect.width/2.f,
                       LocalTextRect.top + LocalTextRect.height/2.f);
//        Text.setPosition(Scale, Scale);
        Text.setColor(sf::Color::White);
    }

    lexeme_node() : lexeme_node(NullLexeme, 50.f) {}
    
    void sf::Drawable::draw(sf::RenderTarget& Target, sf::RenderStates States) const
    {
        States.transform *= getTransform();
        Target.draw(Outline, States);
        Target.draw(Text, States);
    }
};

enum class lexeme_relationship_type
{
    undefined,
    does, // NOTE(tyler): A noun lexeme *does* a verb lexeme. e.g., a merchant *does* selling; a merchant sells.
};

class lexeme_relationship_line : public sf::Drawable, public sf::Transformable
{
private:
//    lexeme_relationship& mRelationship;
    
    sf::RectangleShape mLine;
public:
    lexeme_relationship_line() : mLine(sf::Vector2f(1.f, 4.f))
    {
        mLine.setFillColor(sf::Color::Red);
    }
    
    void sf::Drawable::draw(sf::RenderTarget& Target, sf::RenderStates States) const
    {
        States.transform *= getTransform();
        Target.draw(mLine, States);
    }
};
// Connect specific noun modifiers with specific verbs? (e.g., a "chemical merchant": chemical connects to "sell")

// Noun modifiers can describe a certain characteristic of a noun. Link those explicitly? 
    
float Distance(const sf::Vector2f& Source, const sf::Vector2f& Dest)
{
    return pow(pow(Dest.x - Source.x, 2.f) + pow(Dest.y - Source.y, 2.f), 0.5f);
} 

float VectorLength(const sf::Vector2f& Vector)
{
    return Distance(sf::Vector2f(0.f, 0.f), Vector);
}

sf::Vector2f DirectionVector(sf::Vector2f Source, sf::Vector2f Dest)
{
    return (Dest - Source) / Distance(Dest, Source);
}

float VectorAngle(sf::Vector2f Vector)
{
    sf::Vector2f ReferenceVector(1.f, 0.f);

    return atan2(Vector.y, Vector.x);
}

class visual_grammar;

class grammar
{
private:

protected:
    struct lexeme_relationship
    {
        uint16_t SourceID;
        uint16_t TargetID;
        lexeme_relationship_type Type;

        lexeme_relationship(uint16_t SourceID, uint16_t TargetID, lexeme_relationship_type Type)
                : SourceID(SourceID), TargetID(TargetID), Type(Type) {}
    };

    std::unordered_map<uint16_t, lexeme> mLexemes;
    std::unordered_map<uint16_t, lexeme_relationship> mLexemeRelationships;
    
    uint16_t mLexemeRelationshipCount;
    
public:
    grammar() : mLexemeRelationshipCount(-1) {}
    
    void CreateLexeme(uint16_t LexemeID, lexeme_type Type, std::string Lemma, inflection_table& InflectionTable)
    {
        mLexemes.emplace(LexemeID, lexeme(Type, Lemma, InflectionTable));;
    }

    void CreateLexemeRelationship(uint16_t SourceLexemeID, 
                                  lexeme_relationship_type RelationshipType,
                                  uint16_t TargetLexemeID)
    {
        mLexemeRelationshipCount++;
        mLexemeRelationships.emplace(mLexemeRelationshipCount,
                                     lexeme_relationship(SourceLexemeID,
                                                         TargetLexemeID,
                                                         RelationshipType));
    }
};

class visual_grammar : public grammar, public sf::Drawable, public sf::Transformable
{
private:
    std::unordered_map<uint16_t, lexeme_node> mLexemeNodes;
    std::unordered_map<uint16_t, lexeme_relationship_line> mLexemeRelationshipLines;

    lexeme_node* mSelectedNode;
public:
    visual_grammar() : mSelectedNode(nullptr) {}
    
    void CreateLexeme(uint16_t LexemeID, lexeme_type Type, std::string Lemma, inflection_table& InflectionTable)
    {
        grammar::CreateLexeme(LexemeID, Type, Lemma, InflectionTable);

        mLexemeNodes.emplace(LexemeID, lexeme_node(lexeme(Type, Lemma, InflectionTable), 50.f));
    }
    
    void CreateLexemeRelationship(uint16_t SourceLexemeID, 
                                  lexeme_relationship_type RelationshipType,
                                  uint16_t TargetLexemeID)
    {
        grammar::CreateLexemeRelationship(SourceLexemeID, RelationshipType, TargetLexemeID);

        mLexemeRelationshipLines.emplace(mLexemeRelationshipCount, lexeme_relationship_line());
    }
    
    void ProcessEvent(sf::Event Event)
    {
            if(Event.type == sf::Event::MouseButtonPressed)
            {
                if(Event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f MousePosition(Event.mouseButton.x, Event.mouseButton.y);
                    
                    for(const auto& kv : mLexemeNodes)
                    {
                        const lexeme_node& Node = kv.second;
                        
                        float distance = Distance(MousePosition, Node.getPosition());
                        
                        if(distance <= Node.Scale)
                        {
                            mSelectedNode = &const_cast<lexeme_node&>(Node);
                        }
                    }
                }
                if(Event.mouseButton.button == sf::Mouse::Right)
                {
//                    CreateLexeme(
                }
            }
            if(Event.type == sf::Event::MouseButtonReleased)
            {
                if(Event.mouseButton.button == sf::Mouse::Left)
                {
                    mSelectedNode = nullptr;
                }
            }        
    }

    void Update(sf::RenderWindow& Window)
    {
        if(mSelectedNode != nullptr)
        {
            mSelectedNode->setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition(Window)));
        }
        for(auto&& kv : mLexemeRelationshipLines)
        {
            uint16_t LexemeRelationshipID = kv.first;
            lexeme_relationship_line& LRLine = kv.second;
            lexeme_relationship& LR = mLexemeRelationships.at(kv.first);

            const sf::Vector2f SourceNodePosition = mLexemeNodes[LR.SourceID].getPosition();
            const sf::Vector2f TargetNodePosition = mLexemeNodes[LR.TargetID].getPosition();
            const sf::Vector2f EdgeDirectedVector = DirectionVector(SourceNodePosition, TargetNodePosition);
            const sf::Vector2f LineStartPosition = SourceNodePosition + EdgeDirectedVector * 50.f;
            const sf::Vector2f LineEndPosition = TargetNodePosition - EdgeDirectedVector * 50.f;

            float NodeDistance = Distance(SourceNodePosition, TargetNodePosition);

            lexeme_relationship_line& LRL = mLexemeRelationshipLines[LexemeRelationshipID];
            
            LRL.setPosition(LineStartPosition);
            LRL.setRotation(VectorAngle(EdgeDirectedVector) * 180.f / 3.14159f);
            if(NodeDistance >= 100.f)
            {
                LRL.setScale(Distance(LineStartPosition, LineEndPosition), 1.f);
            }
            else
            {
                LRL.setScale(0.f, 1.f);
            }
        }
    }
    
    void sf::Drawable::draw(sf::RenderTarget& Target, sf::RenderStates States) const
    {
        States.transform *= getTransform();

        for(const auto& kv : mLexemeNodes)
        {
            Target.draw(kv.second, States);
        }
        for(const auto& kv : mLexemeRelationshipLines)
        {
            Target.draw(kv.second, States);
        }
    }
};

int main()
{
    CourierFont.loadFromFile("assets/ABZ.ttf");
    
    // NOTE(tyler): Window setup
    sf::ContextSettings Settings;
    Settings.antialiasingLevel = 8;
    unsigned int WindowWidth = 1280;
    unsigned int WindowHeight = 720;
    sf::RenderWindow Window(sf::VideoMode(WindowWidth, WindowHeight), "", sf::Style::Default, Settings);
    Window.setFramerateLimit(60);

    // NOTE(tyler): Other stuff setup
    inflection_table RegularIT("inflection tables/conjugation/regular.csv");

    visual_grammar Grammar;
    Grammar.CreateLexeme(0, lexeme_type::noun, "merchant", RegularIT);
    Grammar.CreateLexeme(1, lexeme_type::verb, "sell", RegularIT);
    Grammar.CreateLexemeRelationship(0, lexeme_relationship_type::does, 1);

    sfg::SFGUI SFGUI;
    
    auto TextBox = sfg::Entry::Create("test text");
    auto Box = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
    Box->Pack(TextBox);

    auto GUIWindow = sfg::Window::Create();
    GUIWindow->SetTitle( "Hello World example" );
    GUIWindow->Add( Box );


    sfg::Desktop Desktop;
    Desktop.Add(GUIWindow);

//    sfg::
    
    sf::Event Event;
    
    while(Window.isOpen())
    {
        while(Window.pollEvent(Event))
        {
            Grammar.ProcessEvent(Event);
            Desktop.HandleEvent(Event);

            if(Event.type == sf::Event::Closed)
            {
                Window.close();
            }
        }
        
        // NOTE(tyler): Update step
        Grammar.Update(Window);
        Desktop.Update(1.f);
        
        // NOTE(tyler): Draw step
        Window.clear();

        Window.draw(Grammar);
        SFGUI.Display(Window);
        
        Window.display();
    }

    return 0;
}
