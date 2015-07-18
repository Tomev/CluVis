#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H



class generalSettings
{
public:
    //Members
        //Constans
            //Data type ID

    const static int RSES_RULES_ID = 0;

        //Variables

    int dataTypeID = 0;
    int objectsNumber = 0;
    int stopCondition = 1;

    //Methods

    generalSettings();
    ~generalSettings();
};

#endif // GENERALSETTINGS_H
