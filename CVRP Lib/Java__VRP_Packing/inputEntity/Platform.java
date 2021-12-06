package com.quadtalent.quadx.inputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;


@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Platform {
    private String platformCode;
    private boolean isMustFirst;
}
