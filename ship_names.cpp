//
//  ship_names.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/1/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "ship_names.hpp"
#include "RMeth.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <regex>
using namespace std;

const std::vector<std::string> shipname_type_by_class = {
    "MERCHANT SHIPS", "PIRATES",        "WARSHIPS",
    "MERCHANT SHIPS", "MERCHANT SHIPS", "MERCHANT SHIPS",
    "WARSHIPS",       "WARSHIPS",       "MERCHANT SHIPS",
    
    "MERCHANT SHIPS", "WARSHIPS",       "WARSHIPS",
    "MERCHANT SHIPS", "MERCHANT SHIPS", "MERCHANT SHIPS",
    "WARSHIPS",       "WARSHIPS",       "MERCHANT SHIPS",
    
    "MERCHANT SHIPS", "WARSHIPS",       "WARSHIPS",
    "MERCHANT SHIPS", "MERCHANT SHIPS", "MERCHANT SHIPS",
    "WARSHIPS",       "WARSHIPS",       "MERCHANT SHIPS"
};

string last_flag = "";
void save_last_flag(string flag) { last_flag = flag;   }

int last_shiptype = 0;
string save_last_shiptype(const PstLine & i) {
    last_shiptype = stoi(i.value);
    return "";
}

// The data is taken from the lang0.FPK file by unpacking shipnames_enu.txt
const string dump_of_shipnames = R"(
#SPANISH MERCHANT SHIPS
Antonio, M
Nina, M
Santa Maria, M
Hispaniola, M
Nuestra Senora de la Popa, M
Maria Galande, M
Don Fernando II, M
Mexicano, M
San Antonio, M
Nuestra Senora de Cabo, M
Sangre de Christo, M
La Gaviota, M
El Viento Bonita, M
Santo Jose de Cadiz, M
Nuestra Senora de Madrid, M
Nuestra Senora, M
El Dios Vivo, M
La Hispaniola, M
La Espana, M
Rey Carlos, M
Santo Domingo, M
Castilla Nueva, M
Sierra Nevada, M
Cataluna, M
Aragon, M
Pamplona, M
Costa Verde, M
La Mancha, M
Comerciante Afortunado, M
Grenada, M
La Villa Hermosa, M
Barcelona, M
Galicia, M
Marinero Liso, M
Novia de Ruborizacion, M
Infante, M
Vieja Senora, M
Potro, M
Caballo Rapido, M
Cosecha Grande, M
#SPANISH WARSHIPS
Nuestra Senora de la Concepcion, M
Philip, M
San Geronimo, M
Magdalen, M
Santa Louisa, M
Marquesa, M
Maria Asumpta, M
Santissima Trinidad, M
San Nicolas, M
San Josef, M
Salvador del Mundo, M
San Ysidro, M
Principe de Asturias, M
Conde de Regla, M
Oriente, M
Atlante, M
Soberano, M
Infante de Pelayo, M
San Ildephonso, M
San Pablo, M
Neptuna, M
San Domingo, M
Terrible, M
Guadelupe, M
Hermoine, M
Santa Brigida, M
Santa Teresa, M
Thetis, M
Del-Carmen, M
Florentina, M
Real, M
San Hermenegildo, M
Perla, M
Gamo, M
Amfitrite, M
Medea, M
Fama, M
Clara, M
Mercedes, M
Santa Gertruyda, M
#SPANISH PIRATES
Diabolito, M
El Dragon, M
Manchado de Sangre, M
Marinero Roja, M
La Abominacion, M
La Perla Negra, M
El Caballo Negro, M
El Tiburon Hambriento, M
Inundacion Del Oro, M
Tormenta Roja, M
Venganza, M
Craneo Negro, M
Huracan, M
Hombre Ahogado, M
El Gran Amante, M
El Hombre Colgado, M
La Serpiente , M
La Serpiente Enojada, M
Alta Marea, M
Puesta Del Sol Roja, M
Rumor, M
Afortunado, M
Girasol, M
Nave De la Plaga, M
La Mermaid, M
Puno Sangriento, M
Comedor de Fuego, M
Tigre Flotante, M
Rato Enojada, M
El Asesino, M
Deseo Del Corazon, M
Sangre Del Corazon, M
El Antilope Corriente, M
El Buitre Feliz, M
El Leon, M
Valencia, M
Aburo Grande, M
Pequeno Ranunculo, M
El Cocodrilo, M
El Pirata Enojado, M
#ENGLISH MERCHANT SHIPS
Betsy, M
Bingham, M
Blue Heron, M
Blue Skies, M
Carrie Nelson, M
Cheshire, M
Dolphin, M
Donkey Cart, M
Dove, M
Drury Lane, M
Dullard's Walk, M
England, M
Flame, M
Flying Hart, M
French Leave, M
Game Cock, M
Half-tide, M
Iris, M
Kingfisher, M
Liverpool, M
London, M
Lovely Mary, M
Michael York, M
Neep, M
Nottingham, M
Nutmeg, M
Plover's Egg, M
Red Boar, M
Right Whale, M
Royal Purchase, M
Sea Leopard, M
Sea Otter, M
Sea Rose, M
Seafarer, M
Snipe, M
Spark, M
Swallow, M
Tabitha, M
Trent, M
Ushant, M
Victoria's Enigma, M
Yorkshire, M
Zenobia, M
#ENGLISH WARSHIPS
Admiral Fell, M
Atlantica, M
Blue Sailor, M
Camel, M
Courage, M
Courageous, M
Dawn Treader, M
Defense, M
Distinguished, M
Duke of Kent, M
Duke of Richmond, M
Duke of York, M
Fearless, M
Fox Hound, M
Invincible, M
Irrepressible, M
Jarvis, M
King Charles, M
King Henry, M
King Richard, M
Liberty, M
Lion, M
Lord Cheltenham, M
Lord Hawkins, M
Night Swallow, M
Oceania, M
Omniverous, M
Panther, M
Queen Elizabeth, M
Queen Mary, M
Red Harbor, M
Reliant, M
Resilient, M
Revenge, M
Sea Tiger, M
Shark, M
Sprite, M
Sunset, M
Surprise, M
Tracer, M
Undauntable, M
Urgent, M
Victory, M
#ENGLISH PIRATES
Adventure, M
Adventure Galley, M
Adventure Prize, M
Bachelor's Delight, M
Black Joke, M
Blessing, M
Blood Debt, M
Bloody Delight, M
Cassandra, M
Charles, M
Childhood, M
Delight, M
Delivery, M
Desire, M
Fancy, M
Flying Dragon, M
Flying Horse, M
Flying King, M
Fortune, M
Gift, M
Golden Hind, M
Good Fortune, M
Happy Delivery, M
Liberty, M
Little Ranger, M
Loyal Fortune, M
Mary Anne, M
Night Rambler, M
Queen Anne's Revenge, M
Ranger, M
Revenge, M
Rising Sun, M
Rover, M
Royal Fortune, M
Royal James, M
Scowerer, M
Sea King, M
Snap Dragon, M
Speaker, M
Speedy Return, M
Sudden Death, M
Tiger, M
Victory, M
#FRENCH MERCHANT SHIPS
Maiden, M
St. Pierre, M
L'Union, M
Rouparelle, M
St. Jean, M
Duquesne, M
Creole, M
Clorinde, M
Vertu, M
Baionnaise, M
Champs Elysees, M
Notre Dame, M
La Coucher Du Soleil, M
Le Retour Heureux, M
Honnetete, M
Elephant, M
Cacao, M
Les Caribes, M
Aphrodite, M
Aquitaine, M
Pyrenees, M
Marseille, M
Toulon, M
St-Tropez, M
Leucate, M
Leon, M
La Rochelle, M
Ile de Re, M
St. Vincent, M
Belle Ile, M
Nantes, M
Poitres, M
Versailles, M
Le Negociant Honnete, M
Rose, M
La Tulipe, M
La Reunion, M
L'albatros, M
La Baleine, M
La Mouette Affamee, M
#FRENCH WARSHIPS
Sainte Rose, M
Le Hasardeux, M
L'Esperance, M
Le Soleil Royal, M
L'Invention, M
Bucentaure, M
Redoubtable, M
L'Orient, M
Leander, M
Forte, M
Junon, M
Alceste, M
Courageuse, M
Charente, M
Preneuse, M
Prudente, M
Vestale, M
Brune, M
Guillaume, M
Genereux, M
Concorde, M
Diane, M
Vengeance, M
Desiree, M
Pallas, M
Carthagenoise, M
Medee, M
Venus, M
Saint Antoine, M
Causse, M
Egyptienne, M
Africaine, M
Justice, M
Carrere, M
Bravoure, M
Chiffonne, M
Dedaigneuse, M
Regeneree, M
Succes, M
Surveillante, M
#FRENCH PIRATES
Trompeuse, M
La Confiance, M
Blanco, M
La Trompeuse, M
La Nouvelle Trompeuse, M
Revanche, M
Chien Rouge, M
La Vengeance, M
Le Guerrier Heureux, M
L'Epine Noire, M
Le Bourreau, M
Le Marin Mort, M
Tortuga, M
Le Requin Blanc, M
La Main Sanglante, M
La Sabre Affame, M
Desolation, M
Desepoir, M
La Fregate Noire, M
Le Traitre, M
Le Vautour, M
La Dame Noire, M
Le Squelette, M
Le Lion, M
L'Homme Mort, M
Le Cadavre, M
Le Marin Ivre, M
Le Tigre, M
Le Meutrier, M
Amiens, M
Cascogne, M
Le Poisson Affame, M
La Marin Chanceux, M
Le Chariot, M
Le Volcan, M
Novembre Fonce, M
Le Judas, M
La Celebration, M
Le Chat, M
Le Poignard, M
#DUTCH MERCHANT SHIPS
Zuider Zee, M
Ellen, M
Ida, M
Meta-Jan, M
Wasa, M
Amsterdam, M
Copenhaagen, M
De Tulp, M
Radijs, M
Bruinvis, M
Konijn, M
Parel, M
Liesbeth Hudson, M
Renate , M
Juliana, M
Kattegat, M
Skagerrak, M
Zeeland, M
Noord Holland, M
Magdalena, M
Ijsselmeer, M
Naaldwijk, M
Roosendaal, M
Tilburg, M
Eindhoven, M
Arnhem, M
Rijn, M
Apeldoorn, M
Hengelo, M
Vliestroom, M
Vlieland, M
Lelystad, M
De Vrede, M
Rotterdam, M
Utrecht, M
Het Zilveren Overzees, M
De Zoon van Baker, M
Het Mooie Meisje, M
De Mooie Dame, M
Heilge Mary, M
#DUTCH WARSHIPS
Hollandtschen Tuyn, M
Dromedarus, M
Cerberus, M
De Ruyter, M
Guelderland, M
Leyden, M
Utrecht, M
Vervachten, M
Batavier, M
Beschermer, M
Broederchap, M
Constitutie, M
Duifze, M
Expeditie, M
Hector, M
Mars, M
Amphritrite, M
Ambuscade, M
Helden, M
Minerve, M
Alarm, M
Pollock, M
Venus, M
Zealand, M
Holstein, M
Proserpine, M
Stier, M
Buffel, M
Guinea, M
De Otter, M
Zeus, M
De Hamer Van Thor, M
Resoluut, M
Onverschrokken, M
De Onderneming, M
Ares, M
Odin, M
De Prinses Bruid, M
Inspanning, M
Hermes, M
#DUTCH PIRATES
Schorpioen, M
De Gokker, M
Het Web Van De Spin, M
Os, M
Tijger, M
De Overzeese Slang, M
De Overseese Heks, M
De Zwarte Hand, M
De Zwarte Slang, M
Bloedig Nam Toe, M
Groenland, M
De Rode Prins, M
Het Rode Onweer, M
Vreselijk, M
Wraak, M
De Moordenaar, M
De Veroordeelde Mensen, M
De Zwarte Vloek, M
De Prinses, M
De Gek, M
De Leeuw, M
De Adder, M
De Hongerige Wolf, M
Gertrude, M
Agnes, M
Anna, M
Het Zwarte Bos, M
Rode Oogst, M
Het Achtervolgde Schip, M
Rood Nam Toe, M
Wit Nam Toe, M
Het Mooie Meisje, M
De Rode Kus, M
De Wraak Van De Dwaas, M
Zwarte Waltz, M
Edwin Dakken, M
Otto Smoots, M
Ragnarok, M
De Notemuskaat, M
De Piraat, M
#PIRATE MERCHANT SHIPS
Admiral Fell, M
Atlantica, M
Blue Sailor, M
Camel, M
Courage, M
Courageous, M
Dawn Treader, M
Defense, M
Distinguished, M
Duke of Kent, M
Duke of Richmond, M
Duke of York, M
Fearless, M
Fox Hound, M
Invincible, M
Irrepressible, M
Jarvis, M
King Charles, M
King Henry, M
King Richard, M
Liberty, M
Lion, M
Lord Cheltenham, M
Lord Hawkins, M
Night Swallow, M
Oceania, M
Omniverous, M
Panther, M
Queen Elizabeth, M
Queen Mary, M
Red Harbor, M
Reliant, M
Resilient, M
Revenge, M
Sea Tiger, M
Shark, M
Sprite, M
Sunset, M
Surprise, M
Tracer, M
Undauntable, M
Urgent, M
Victory, M
#PIRATE WARSHIPS
Adventure, M
Adventure Galley, M
Adventure Prize, M
Bachelor's Delight, M
Black Joke, M
Blessing, M
Blood Debt, M
Bloody Delight, M
Cassandra, M
Charles, M
Childhood, M
Delight, M
Delivery, M
Desire, M
Fancy, M
Flying Dragon, M
Flying Horse, M
Flying King, M
Fortune, M
Gift, M
Golden Hind, M
Good Fortune, M
Happy Delivery, M
Liberty, M
Little Ranger, M
Loyal Fortune, M
Mary Anne, M
Night Rambler, M
Queen Anne's Revenge, M
Ranger, M
Revenge, M
Rising Sun, M
Rover, M
Royal Fortune, M
Royal James, M
Scowerer, M
Sea King, M
Snap Dragon, M
Speaker, M
Speedy Return, M
Sudden Death, M
Tiger, M
Victory, M
#PIRATE PIRATES
Adventure, M
Adventure Galley, M
Adventure Prize, M
Bachelor's Delight, M
Black Joke, M
Blessing, M
Blood Debt, M
Bloody Delight, M
Cassandra, M
Charles, M
Childhood, M
Delight, M
Delivery, M
Desire, M
Fancy, M
Flying Dragon, M
Flying Horse, M
Flying King, M
Fortune, M
Gift, M
Golden Hind, M
Good Fortune, M
Happy Delivery, M
Liberty, M
Little Ranger, M
Loyal Fortune, M
Mary Anne, M
Night Rambler, M
Queen Anne's Revenge, M
Ranger, M
Revenge, M
Rising Sun, M
Rover, M
Royal Fortune, M
Royal James, M
Scowerer, M
Sea King, M
Snap Dragon, M
Speaker, M
Speedy Return, M
Sudden Death, M
Tiger, M
Victory, M
)";

map <string, vector<string>> shipnames_list;

void load_pirate_shipnames() {
    // Load all of the shipnames from the big string above and use them to populate
    // a map so that shipnames can be looked up for decoding.
    stringstream data = stringstream(dump_of_shipnames);
    string aline;
    string shipclass = "";
    while(getline(data,aline)) {
        if (regex_search(aline, regex("^#"))) {
            shipclass = regex_replace(aline, regex("#"), "");
            shipnames_list[shipclass] = {};
        } else if (shipclass != "") {
            // Discard gender of ships (, M).
            // (That was in the dump from SMP because SMP supports languages other than English.)
            string name = regex_replace(aline, regex(",.*"),"");
            shipnames_list[shipclass].push_back(name);
        }
    }
}

string translate_shipname(const PstLine & i) {
    if (last_flag == "") { return "NIL"; }
    if (last_shiptype < 0 ) { return "NIL"; }

    // Assemble the shipname_group a combination of the flag and shipname_group
    // to know which list of shipnames to use - merchant, warship, or pirate.
    string shipname_group = last_flag + " " + shipname_type_by_class.at(last_shiptype);
    std::transform(shipname_group.begin(), shipname_group.end(),shipname_group.begin(), ::toupper);
    
    // All pirate ships get English Pirate shipnames, and all Indian ships get Spanish names.
    // Jesuit ships sail under Spanish flags, so they get Spanish names but don't need an adjustment
    // to shipname_group.
    shipname_group = regex_replace(shipname_group, regex("^PIRATES.*"), "ENGLISH PIRATES");
    shipname_group = regex_replace(shipname_group, regex("^INDIAN.*"), "SPANISH MERCHANT SHIPS");
    
    if (shipnames_list.count(shipname_group)) {
        vector<string> & list = shipnames_list.at(shipname_group);
        if (i.v >= 0 && i.v < list.size()) {
            if (list.at(i.v).size() > 0) {
                return list.at(i.v);
            }
        }
    } else {
        throw logic_error("Bad shipname_group " + shipname_group);
    }
    return "";  
}
