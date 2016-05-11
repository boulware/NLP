#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>
#include <cstdint>

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
    friend class grammar;
private:
    lexeme_type mType;
    std::string mLemma;
    inflection_table& mInflectionTable;

    std::vector<lexeme*> mInteractions;
    
    lexeme(const lexeme& Source) : mLemma(Source.mLemma), mInflectionTable(Source.mInflectionTable) {}
    lexeme(lexeme_type Type, std::string Lemma, inflection_table& InflectionTable) : mType(Type), mLemma(Lemma), mInflectionTable(InflectionTable) {}

    std::string Inflect(feature_structure FS)
    {
        return mInflectionTable.QueryInflection(FS).Inflect(mLemma);
    }
};

struct LexemeKeyHasher
{
    std::size_t operator()(const lexeme& k) const
    {
        using std::size_t;
        using std::hash;
        using std::string;

        return (
            (hash<std::size_t>()(static_cast<std::size_t>(k.mType))) ^
            (hash<std::string>()(static_cast<std::string>(k.mLemma)) << 8)
                );
    }
};

class lexeme_node : public sf::Drawable, public sf::Transformable
{
private:
    sf::CircleShape Outline;
    sf::Text Text;

    lexeme& Lexeme;
public:
    float Scale;
    
    lexeme_node(lexeme Lexeme, float Scale) : Lexeme(Lexeme), Scale(Scale), Outline(Scale), Text(Lexeme.mLemma, CourierFont, 50.f)
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
    
    void sf::Drawable::draw(sf::RenderTarget& Target, sf::RenderStates States) const
    {
        States.transform *= getTransform();
        Target.draw(Outline, States);
        Target.draw(Text, States);
    }
};

enum class lexeme_relationship_type
{
    does, // NOTE(tyler): A noun lexeme *does* a verb lexeme. e.g., a merchant *does* selling; a merchant sells.
};

class lexeme_relationship
{
private:
    lexeme* mSource;
    lexeme* mTarget;

    lexeme_relationship_type mType;
public:
    lexeme_relationship(lexeme* Source, lexeme* Target, lexeme_relationship_type Type)
            : mSource(Source), mTarget(Target), mType(Type) {}
};

class lexeme_relationship_line : public sf::Drawable, public sf::Transformable
{
private:
    lexeme_relationship& mRelationship;
    
    sf::RectangleShape mLine;
public:
    lexeme_relationship_line(lexeme_relationship Relationship) : mLine(sf::Vector2f(10.f, 4.f)), mRelationship(Relationship)
    {
        mLine.setFillColor(sf::Color::Red);
    }
    
    void sf::Drawable::draw(sf::RenderTarget& Target, sf::RenderStates States) const
    {
        States.transform *= getTransform();
        Target.draw(mLine);
    }
};

/*
class thing // NOTE(tyler): A very existential meaning of "thing"; not necessarilly a tangible object. Includes objects, actions, descriptors, etc..
// NOTE(tyler): Primarily a way to connect lexical meaning (lexeme) to something in the real world in a generic way (to be inherited).
{
private:
    lexeme mLexeme;
public:
    thing(lexeme Lexeme) : mLexeme(Lexeme) {}
};
*/
inflection_table NullInflectionTable;
//lexeme NullLexeme(lexeme_type::undefined, "_null_", NullInflectionTable);
/*
class action// : public thing
{
private:
    lexeme mLexeme;
public:
    action(lexeme& Lexeme) : mLexeme(Lexeme) {}
};

class object// : public thing
{
private:
    lexeme mLexeme;
public:
    object() : mLexeme(NullLexeme) {}
    object(lexeme& Lexeme) : mLexeme(Lexeme) {}
};
*/ //f
// Connect specific noun modifiers with specific verbs? (e.g., a "chemical merchant": chemical connects to "sell")

// Noun modifiers can describe a certain characteristic of a noun. Link those explicitly? 
    
float Distance(const sf::Vector2f& Source, const sf::Vector2f& Dest)
{
    return pow(pow(Dest.x - Source.x, 2.f) + pow(Dest.y - Source.y, 2.f), 0.5f);
}

void CreateLexeme(std::vector<lexeme>& LexemeVector, std::vector<lexeme_node>& NodeVector, lexeme& Lexeme)
{
    LexemeVector.push_back(Lexeme);
    NodeVector.push_back(lexeme_node(Lexeme, 100.f));
}

void CreateLexemeRelationship(std::vector<lexeme_relationship>& LexemeRelationships,
                              std::vector<lexeme_relationship_line>& LexemeRelationshipLines,
                              std::vector<lexeme>& LexemeVector, 
                              unsigned int SourceLexemeIndex, unsigned int TargetLexemeIndex,
                              lexeme_relationship_type LexemeRelationshipType)
{
    lexeme_relationship LR(&LexemeVector[SourceLexemeIndex], &LexemeVector[TargetLexemeIndex], LexemeRelationshipType);
    LexemeRelationships.push_back(LR);
    LexemeRelationshipLines.push_back(lexeme_relationship_line(LR));
}

class grammar
{
private:
    // NOTE(tyler): The mechanical aspects of the grammar.
    std::vector<lexeme> mLexemes;
    std::vector<lexeme_relationship> mLexemeRelationships;

    // NOTE(tyler): Visual representation of the grammar.
    std::vector<lexeme_node> mLexemeNodes;
    std::vector<lexeme_relationship_line> mLexemeRelationshipLines;
public:
    grammar() {}

    void CreateLexeme(lexeme_type Type, std::string Lemma, inflection_table& InflectionTable)
    {
        mLexemes.push_back(lexeme(Type, Lemma, InflectionTable));
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

    std::vector<lexeme> Lexemes;
    std::vector<lexeme_node> LexemeNodes;
    std::vector<lexeme_relationship> LexemeRelationships;
    std::vector<lexeme_relationship_line> LexemeRelationshipNodes;

    CreateLexeme(Lexemes, LexemeNodes, lexeme(lexeme_type::noun, "merchant", RegularIT));
    CreateLexeme(Lexemes, LexemeNodes, lexeme(lexeme_type::verb, "sell", RegularIT));
    CreateLexemeRelationship(LexemeRelationships, LexemeRelationshipNodes, Lexemes, 0, 1, lexeme_relationship_type::does);

    lexeme_node* SelectedNode = nullptr;
    
    while(Window.isOpen())
    {
        sf::Event Event;
        while(Window.pollEvent(Event))
        {
            if(Event.type == sf::Event::MouseButtonPressed)
            {
                if(Event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f MousePosition(Event.mouseButton.x, Event.mouseButton.y);
                    
                    for(auto& e : LexemeNodes)
                    {
                        float distance = Distance(MousePosition, e.getPosition());
                        
                        if(distance <= e.Scale)
                        {
                            SelectedNode = &e;
                        }
                    }
                }
            }
            if(Event.type == sf::Event::MouseButtonReleased)
            {
                if(Event.mouseButton.button == sf::Mouse::Left)
                {
                    SelectedNode = nullptr;
                }
            }
            if(Event.type == sf::Event::Closed)
            {
                Window.close();
            }
        }
        
        // NOTE(tyler): Update step
        if(SelectedNode != nullptr)
        {
            SelectedNode->setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition(Window)));
        }

        
        // NOTE(tyler): Draw step
        Window.clear();

        for(auto& e : LexemeNodes)
        {
            Window.draw(e);
        }
        for(auto& e : LexemeRelationshipNodes)
        {
            Window.draw(e);
        }
        
        Window.display();
    }

    return 0;
}
