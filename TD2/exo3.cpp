/** Auteur : Mathys JAM TD1A
 * Je vous recommande d'essayer par vous meme avant de lire les corrections
 * Cette correction n'est qu'a titre indicatif, et a ete ecrite par un eleve
 * 
 * J'uttilise certaines notions avancés qui ne seront peut etre pas vu en cours
 * Cependant, elles correspondent a une utilisation correcte du c++
 * 
 * 
 * 
 * N'hesitez pas a me contactez
 * Discord : Thukisdo#8050
 */

#include <iostream>
#include <memory>
#include <string.h>
#include "CString.hpp"

// =====================================================================
/* A partir d'ici, j'utilise deux nouvelles notions assez avancés
 - les template
 - la sémantique de mouvement


====== Les templates
    Je vous invite a vous referez a internet
    il s'agit de creer des "template"
    Ce sont des fonctions qui peuvent accepter plusieurs type d'arguments, mais qui ne sont declarer qu'une seule fois

    template<typename t>
    void f(T const& x)
    {
        T c = x;
    }

    ici, la fonction f accepte n'importe quel arguments pour lequel operator= est défini
    Cela permet d'éviter de faire f(int x), f(double x), f(long x), f(char x) ...

======== La sémantique de mouvement

    Pour eviter de rentrez dans les détails :
    On a vu en cours la notion de référence (int& a par exemple)
    Ici, nous allons utiliser des reference a gauche (int&& a)

    une reference a gauche pointe vers une "variable temporaire"
    par exemple

    void f(int& a);
    void f(int&& a);

    int a = 5;
    f(a); ici, on apelle f(int& a) car a est une variable classique
    f(5); ici, on apelle f(int&& a) car 5 est une variable temporaire

    A quoi sa sert ?

    cela permet d'accelerer certaines opérations et économiser de la mémoire par exemple :

    CString a("Je suis une variable classique");
    CString b = CString("Je suis une variable temporaire");
        - on voit ici qu'on doit allouer de la mémoire pour la string
        - puis allouer de la mémoire dans b pour copier
        - puis recopier la variable temporaire dans b
        - puis detruire la variable temporaire et désallouer sa mémoire
    
    en faisant
    CString::operator=(CString& str)
    {
        ... //copie
    }
    CString::operator=(CString&& str)
    {
        this.pointeur = str.pointeur;
        str.pointeur = NULL;
    }
    CString sera capable de comprendre que str est une variable temporaire
    et il pourra donc lui "voler" sa mémoire (en prenant son pointeur)
    Plutot que de devoir reallouer un tableau

    CString a("Je suis une variable");
    CString b = std::move(a) // ici, a cause de std::move, 'a' sera consideré comme variable temporaire
    // et sa mémoire sera donc volé

========= Combinaisons des deux

    En combinant les template et les reference, on obtient ce qu'on apelle des "reference universelles"
    qui s'écrivent T&& (Oui, de la meme maniere qu'une variable temporaire)

    void func(int& a)
    {
        ...
    }

    void func(int&& a)
    {
        ...
    }

    template<typename T>
    void f(T&& a)
    {
        func(std::forward<T>(a)); 
    }
    // Ici, on apellera automatiquement la fonctions func qui correspond au type de a (variable normal, ou variable temporaire)

    int a = 5;
    f(a); on fera appel a f(T& a) et donc func(int& a) car a est une variable
    f(5); on fera appel a f(T&& a) et donc func(int&& a) car 5 est une variable temporaire
*/


class Definition
{
private:
    CString clef, def;

public:
    Definition()
    {
    }

    Definition(Definition &val)
    {
        *this = val;
    }

    Definition(Definition &&val)
    {
        *this = val;
    }

    Definition &operator=(Definition &val)
    {
        clef = val.clef;
        def = val.def;
        return *this;
    }

    Definition &operator=(Definition &&val)
    {
        clef = std::move(val.clef);
        def = std::move(val.def);
        return *this;
    }

    // Ici, on apelle le constructeurs de CString approprié
    template <typename T, typename X>
    Definition(T &&clef, X &&def)
    {
        this->clef = std::forward<T>(clef);
        this->def = std::forward<X>(def);
    }

    void afficher() const
    {
        std::cout << "Definition de " << clef << " : " << def << "\n";
    }

    const CString &getClef() { return clef; };
    const CString &getDef() { return def; };
};

class Dictionnaire
{
private:
    struct elem
    {
        elem() : value(), next(nullptr){};
        Definition value;
        std::shared_ptr<struct elem> next;
    };

    std::shared_ptr<elem> begin;
    unsigned m_size;

    // Au lieu de declarer deux fonction
    // insert(Definition const& def)
    // insert(Definition&& def)
    // on declare un template qui va assurer les deux fonctions
    template <typename T>
    bool insert(T &&def)
    {
        // Ici, on doit inserer la définitions dans l'ordre alphabétique

        std::shared_ptr<elem> old = nullptr;
        auto current = begin;
        auto tmp = std::make_shared<elem>();
        // ici, on apelle l'operateur
        // Definition::operator=(Definition const& val)
        // ou
        // Definition::operator=(Definition&& val)
        // dependament de si T est une variable temporaire ou non
        tmp->value = std::forward<T>(def);
        if (begin == nullptr)
        {
            begin = tmp;
            return true;
        }

        while (current)
        {
            if (tmp->value.getClef() == current->value.getClef())
                return false;
            if (tmp->value.getClef() < current->value.getClef())
                break;
            old = current;
            current = current->next;
        }
        tmp->next = current;
        if (old == nullptr)
            begin = tmp;
        else old->next = tmp;
        return true;
    }

    void copy(std::shared_ptr<elem> ptr)
    {
        if (ptr == nullptr)
            return;
        if (!begin)
        {
            begin = std::make_shared<elem>();
            begin->value = ptr->value;
        }
        std::shared_ptr<elem> current = begin;
        while (ptr->next)
        {
            if (!current->next)
                current->next = std::make_shared<elem>();
            current->next->value = ptr->value;
            current = current->next;
            ptr = ptr->next;
        }
    }

    std::shared_ptr<elem> get(CString const& def)
    {
        for (auto current = begin; current != nullptr; current = current->next)
        {
            if (current->value.getClef() == def)
                return current;
        }
        return nullptr;
    }

public:
    Dictionnaire() : m_size(0){};
    Dictionnaire(Dictionnaire const &val) : m_size(0)
    {
        *this = val;
    }

    Dictionnaire(Dictionnaire &&val) : m_size(0)
    {
        *this = val;
    }

    Dictionnaire &operator=(Dictionnaire const &val)
    {
        m_size = val.m_size;
        copy(val.begin);
        return *this;
    }

    Dictionnaire &operator=(Dictionnaire &&val)
    {
        m_size = val.m_size;
        begin = val.begin;
        val.begin = nullptr;
        return *this;
    }

    // on Pourrait déclarer deux fonctions
    // add(Definition const& val)
    // et add(Definition&& val)
    // A la place, on déclare un template qui remplit les deux fonctions
    template <typename T>
    bool add(T &&val)
    {
        if (insert(std::forward<T>(val))) // Ici, on apelle un autre template
        {
            m_size++;
            return true;
        }
        return false;
    }

    const std::shared_ptr<const elem> find(CString const &str)
    {
        return get(str);
    }

    unsigned int getSize()
    {
        return m_size;
    }
};

int main()
{
    Definition homer("Homer", "Buveur de biere");
    Definition bg("bg", "Thukisdo");

    Dictionnaire test;
    test.add(homer); // fait appel au template operator+=(Definition const& val)
    test.add(bg);
    test.add(Definition("Variable temporaire", "C'est moi !"));
    test.add(std::move(homer)); // fait appel au template operator+=(Definition&& val)
    test.find("Homer")->value.afficher();
    test.find("Variable temporaire")->value.afficher();
    test.find("bg")->value.afficher();

    Dictionnaire test2(test);
    test.find("Homer")->value.afficher();
    test.find("Variable temporaire")->value.afficher();
}

// On pourrait énormément améliorer les classes présente dans ce fichier
// Deja on pourrait rajouter des opérateurs a la classe CString pour par exemple concatener deux string
// On pourrait rajouter des fonctions a dictionnaires (afficher une définitions, retirer une définitions, réinitialiser le dictionnaire),
// afficher toutes les définitions ou cherche tout les mots contenant un mot...
// Le code ici fonctionne trés bien mais pourrait etre amélioré si on envisager sérieusement d'utiliser ces classes plus tard