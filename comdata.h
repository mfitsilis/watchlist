int getCompInfo(std::string qry){
        if (result != NULL) r0(result);

        int nRows, nCols;
        result = k(handle, (S)qry.c_str(), (K)0);

        if (!result)
            printf("Network Error\n"), perror("Network"), exit(1);
        if (result->t == -128)
            printf("Server Error %s\n", result->s), kclose(handle), exit(1);

        if (result->t != 99 && result->t != 98) // accept table or dict
        {
            printf("type %d\n", result->t);
            r0(result);//, exit(1);
        }
        else {
            flip = ktd(result); // if keyed table, unkey it. ktd decrements ref count of arg.
                                // table (flip) is column names!list of columns (data)
            columnNames = kK(flip->k)[0];
            columnData = kK(flip->k)[1];
            nCols = columnNames->n;
            nRows = kK(columnData)[0]->n;
            //DO NOT USE r0(result); or columnData cannot be used!
        }

        if (nRows == 0) return -1;
        for (int i = 0; i < nRows; i++) {
            CompInfo.symbol=getItem(0, i);
            CompInfo.name=getItem(1, i);
            CompInfo.sector=getItem(2, i);
            CompInfo.industry=getItem(3, i);
            CompInfo.country=getItem(4, i);
        }
        return 0;
}

int getCompDescr(std::string qry) {
    if (result != NULL) r0(result);

    int nRows, nCols;
    result = k(handle, (S)qry.c_str(), (K)0);

    if (!result)
        printf("Network Error\n"), perror("Network"), exit(1);
    if (result->t == -128)
        printf("Server Error %s\n", result->s), kclose(handle), exit(1);

    if (result->t != 99 && result->t != 98) // accept table or dict
    {
        printf("type %d\n", result->t);
        r0(result);//, exit(1);
    }
    else {
        flip = ktd(result); // if keyed table, unkey it. ktd decrements ref count of arg.
                            // table (flip) is column names!list of columns (data)
        columnNames = kK(flip->k)[0];
        columnData = kK(flip->k)[1];
        nCols = columnNames->n;
        nRows = kK(columnData)[0]->n;
        //DO NOT USE r0(result); or columnData cannot be used!
    }
    
    if (nRows == 0) return -1;
    CompDescr = getItem(1, 0);
    return 0;
}

int getFData(std::string qry) {
    std::vector<int> types;
    std::string Col;
    if (result != NULL) r0(result);
    
    int nRows = 0, nCols = 0;

    result = k(handle, (S)qry.c_str(), (K)0);

    if (!result)
        printf("Network Error\n"), perror("Network"), exit(1);
    if (result->t == -128)
        printf("Server Error %s\n", result->s), kclose(handle), exit(1);

    if (result->t != 99 && result->t != 98) // accept table or dict
    {
        printf("type %d\n", result->t);
        r0(result);//, exit(1);
    }
    else {
        flip3 = ktd(result); // if keyed table, unkey it. ktd decrements ref count of arg.
                              // table (flip) is column names!list of columns (data)
        columnNames3 = kK(flip->k)[0];
        columnData3 = kK(flip->k)[1];
        nCols = columnNames3->n;
        nRows = kK(columnData3)[0]->n;
        //DO NOT USE r0(result); or columnData cannot be used!
    }

    for (int col = 0; col < nCols; col++) {
        K obj = kK(columnData3)[col];
        switch (obj->t) {
        case(6) : { types.push_back(6); }break;   //int
        case(7) : { types.push_back(7); }break;   //long
        case(9) : { types.push_back(9); }break;   //float
        case(11) : { types.push_back(11); }break;  //symbol
        case(14) : { types.push_back(14); }break;  //date
        case(15) : { types.push_back(15); }break; //datetime with msec
        default: {printf("type %d not supported by this client", obj->t); exit(1); }break;
        }
    }

    if (nRows == 0) return -1;
    std::vector<std::string>().swap(FNames); //empty FNames
    std::vector<std::string>().swap(FData); //empty FData
    for (int j = 0; j < nCols; j++) {
        FNames.push_back(kS(columnNames3)[j]);  //for different types?
        Col=getItem0(j, 0, types[j]);
        FData.push_back(Col);
    }
    return  0;
}

class Column {
public:
    std::vector<int> types;
    std::vector<std::string> TNames;
    std::vector<std::string> Col;
    std::vector<std::vector<std::string>> TData;
    int nRows=0, nCols=0;
    Column() {
        loadTdata("0!fdata");
    };
    void loadTdata(std::string qry) {
        if (result != NULL) r0(result);
        
        //result3 = k(handle,(S)qry.c_str(), (K)0);
        result3 = k(handle, "0!fdata", (K)0);

        if (!result3)
            printf("Network Error\n"), perror("Network"), exit(1);
        if (result3->t == -128)
            printf("Server Error %s\n", result3->s), kclose(handle), exit(1);

        if (result3->t != 99 && result3->t != 98) // accept table or dict
        {
            printf("type %d\n", result3->t);
            r0(result3);//, exit(1);
        }
        else {
            flip3 = ktd(result3); // if keyed table, unkey it. ktd decrements ref count of arg.
                                // table (flip) is column names!list of columns (data)
            columnNames3 = kK(flip->k)[0];
            columnData3 = kK(flip->k)[1];
            nCols = columnNames3->n;
            nRows = kK(columnData3)[0]->n;
            //DO NOT USE r0(result); or columnData cannot be used!
        }

        for (int col = 0; col < nCols; col++) {
            K obj = kK(columnData3)[col];
            switch (obj->t) {
            case(6) : { types.push_back(6); }break;   //int
            case(7) : { types.push_back(7); }break;   //long
            case(9) : { types.push_back(9); }break;   //float
            case(11) : { types.push_back(11); }break;  //symbol
            case(14) : { types.push_back(14); }break;  //date
            case(15) : { types.push_back(15); }break; //datetime with msec
            default: {printf("type %d not supported by this client", obj->t); exit(1); }break;
            }
        }

        for (int j = 0; j < nCols; j++) {
            TNames.push_back(kS(columnNames3)[j]);  //for different types?
            for (int i = 0; i < nRows; i++) { //read col 1st
                Col.push_back(getItem0(j, i,types[j]));
            }
            TData.push_back(Col);
            std::vector<std::string>().swap(Col);
        }
    }
};


class Compinfo {
public:
    std::vector<std::string> symbol;
    std::vector<std::string> name;
    std::vector<std::string> sector;
    std::vector<std::string> industry;
    std::vector<std::string> country;
    std::vector<std::string> description;
    Compinfo() {
        loadcompdata();
    };
    void loadcompdata() {
        if (result != NULL) r0(result);
        char qry[] = "get `:company";

        int nRows, nCols;
        result = k(handle, qry, (K)0);

        if (!result)
            printf("Network Error\n"), perror("Network"), exit(1);
        if (result->t == -128)
            printf("Server Error %s\n", result->s), kclose(handle), exit(1);

        if (result->t != 99 && result->t != 98) // accept table or dict
        {
            printf("type %d\n", result->t);
            r0(result);//, exit(1);
        }
        else {
            flip = ktd(result); // if keyed table, unkey it. ktd decrements ref count of arg.
                                // table (flip) is column names!list of columns (data)
            columnNames = kK(flip->k)[0];
            columnData = kK(flip->k)[1];
            nCols = columnNames->n;
            nRows = kK(columnData)[0]->n;
            //DO NOT USE r0(result); or columnData cannot be used!
        }

        for (int i = 0; i < nRows; i++) {
            symbol.push_back(getItem(0, i));
            name.push_back(getItem(1, i));
            sector.push_back(getItem(2, i));
            industry.push_back(getItem(3, i));
            country.push_back(getItem(4, i));
        }
    }
};

class Compdescr {
public:
    std::vector<std::string> symbol;
    std::vector<std::string> description;
    Compdescr() {
        loadcompdata();
    };
    void loadcompdata() {
        if (result != NULL) r0(result);

        char qry[] = "get `:compinfo"; //should only load once...

        int nRows, nCols;
        result2 = k(handle, qry, (K)0);

        if (!result2)
            printf("Network Error\n"), perror("Network"), exit(1);
        if (result2->t == -128)
            printf("Server Error %s\n", result2->s), kclose(handle), exit(1);

        if (result2->t != 99 && result2->t != 98) // accept table or dict
        {
            printf("type %d\n", result2->t);
            r0(result2);//, exit(1);
        }
        else {
            flip = ktd(result2); // if keyed table, unkey it. ktd decrements ref count of arg.
                                // table (flip) is column names!list of columns (data)
            columnNames = kK(flip->k)[0];
            columnData = kK(flip->k)[1];
            nCols = columnNames->n;
            nRows = kK(columnData)[0]->n;
            //DO NOT USE r0(result); or columnData cannot be used!
        }

        for (int i = 0; i < nRows; i++) {
            symbol.push_back(getItem(0, i));
            description.push_back(getItem(1, i));
        }
    }
};
