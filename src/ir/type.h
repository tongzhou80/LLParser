//
// Created by tlaber on 6/21/17.
//

#ifndef LLPARSER_TYPE_H
#define LLPARSER_TYPE_H


class Type {
public:
    enum TypeID {
        Void,
        Integer,
        Float,
        Double,
        Pointer
    };
private:
    TypeID _type_id;

};

#endif //LLPARSER_TYPE_H
