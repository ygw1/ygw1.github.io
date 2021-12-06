package com.quadtalent.quadx;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;


@Data
@NoArgsConstructor
@AllArgsConstructor
public class Objective {
    public float f1;
    public float f2;

    public boolean isDominated(Objective obj){
        return (f1>=obj.f1 && f2 >= obj.f2);
    }
}
